#ifndef SEMIHOSTING_H_INCLUDED
#define SEMIHOSTING_H_INCLUDED

#pragma once
/**
 * Copyright (c) 2012 Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Linaro Limited nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 */

 /**
 * In accordance with the above license I need to point out that this file
 * was modified in order to port it to C++. There was some adjustments and
 * the operator<< was added.
 */

#define SYS_OPEN	1
#define OPEN_RDONLY	1
#define SYS_CLOSE	2
#define SYS_WRITEC  3
#define SYS_WRITE0	4
#define SYS_WRITE   5
#define SYS_READ	6
#define SYS_READC   7
#define SYS_FLEN	0x0C
#define SYS_GET_CMDLINE	0x15
#define SYS_REPORTEXC	0x18
#define REPORTEXC_REASON_APP_EXIT 0x20026
#define SEMIHOSTING_SVC	0x123456	/* SVC comment field for semihosting */


#ifndef __ASSEMBLER__

namespace stm32plus{

/**
   * Class for using the SemiHosting capabilities.
   * This prevents the use of the printf and friends, saving
   * precious memory.
   */

class SemiHosting{

    private:

        /**
        * \brief Executes the call to the SemiHost
        * \details This is the core of the class.
        * \param id - The function to perform. This will be R0
        * \param arg - The pointer to the arguments to the function. This will be R1
        * \return The new value for R0
        */
        //This function can't be optimized because of the reg0 and reg1 assignments
        static inline int __semi_call(int id, const char *arg)__attribute__((optimize ("O0") )){
            register int reg0 asm("r0") __attribute__((__unused__));
            register int reg1 asm("r1") __attribute__((__unused__));
            reg0=id;
            reg1=(int)arg;

            #if defined(MACH_MPS)
                //profile semihosting is via bpkt
                asm("bkpt    0xab");
            #elif defined(__thumb__)
                //Otherwise, different SVC numbers for ARM or Thumb mode
                asm("bkpt #0xAB");
            #else
               asm("svc 0x00123456");
            #endif
                //asm("mov pc, lr");

            return (reg0);
        }

    public:
        /**
        * \brief Opens a file
        * \details The following table gives the valid values for the integer,
        * and their corresponding ISO C fopen() mode.:

        mode            | 0 | 1  |  2 |	 3  | 4  | 5  | 6  |  7  |	8 |  9 | 10 | 11
        --------------- |-- | -- | -- | --- | -- | -- | -- | --  | -- | -- | -- | --
        ISO C fopen mode| r | rb | r+ |	r+b |  w | wb |	w+ | w+b |	a |	ab | a+ | a+b

        * \param filename - The name of the file
        * \param mode - The open mode.
        * \return A file descriptor (FD)
        * \warning Not tested!
        */
        static int open(const char *filename, int mode);

        /**
        * \brief Closes a file
        * \param fd - The file descriptor
        * \return 0 if successful | -1 if error
        * \warning Not tested!
        */
        static int close(int fd);

        /**
        * \brief Writes a string to the SemiHost
        * \param s - The string to print
        * \return N.A.
        */
        static int puts(const char *s);

        /**
        * \brief Writes to a file using the SemiHost
        * \param fd - The file descriptor
        * \param buffer - The string
        * \param length - The string lenght
        * \return 0 if successful or the number of bytes that are not written, if there is an error.
        */
        static int write(int fd, char *buffer, int length);

        /**
        * \brief Reads from a file using the SemiHost
        * \param fd - The file descriptor
        * \param buffer - The string
        * \param length - The string lenght
        * \return 0 if successful. If it returns the same value as the length the call has failed and EOF is assumed.
        * If it returns a smaller value as the length, the call was partially successful. No error is assumed, but the buffer has not been filled.
        * \warning Not tested!
        */
        static int read(int fd, char *buffer, int length);

        /**
        * \brief Returns the length of a specified file.
        * \param fd - The file descriptor
        * \return the current length of the file object, if the call is successful. -1 if an error occurs.
        * \warning Not tested!
        */
        static int flen(int fd);

        /**
        * \brief Returns the command line used to call the executable, that is, argc and argv.
        * \param buffer - A buffer to write to
        * \param size - The size of the buffer
        * \param length - The length of the buffer after executing
        * \return 0 if successful. -1 if not successful
        * \warning Not tested!
        */
        static int get_cmdline(char *buffer, int size, int *length);


        /**
        * \brief Report an exception to the debugger directly
        * \param reason - The exception. \see http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0471g/Bgbjjgij.html
        * \return Not used
        * \warning Not tested!
        */
        static int reportexc(int reason);

        /**
        * \brief Prints a error message
        * \param message - The error message
        * \return Not used
        * \warning Not tested!
        */
        static void fatal(const char *message);

        /**
        * \brief Create a report for the APP EXIT
        * \return Not used
        * \warning Not tested!
        */
        static void exit(void);
        /**
        * \brief Opens a file (read only) and points to the EOF
        * \param dest - The file carret position
        * \param size - The amount of data readed.
        * \param filename - The filename.
        * \return 0 if successful. -1 if an error occurred
        * \warning Not tested!
        */
        /* semi_load_file: *dest is advanced to point to the end of the loaded data */
        static int load_file(void **dest, unsigned *size, const char *filename);

        /**
        * Operator redefinition to print strings and variables.
        */
        struct Out_{
            Out_& operator<< (const char* s){
               __semi_call(SYS_WRITE0, s);
               return *this;
            }

            Out_& operator<< (const DoublePrecision& val){
                char buffer[25];
                StringUtil::modp_dtoa(val.Value,val.Precision,buffer);
                __semi_call(SYS_WRITE0, buffer);
                return *this;
            }

            Out_& operator<< (double val){
                return operator<<(DoublePrecision(val,DoublePrecision::MAX_DOUBLE_FRACTION_DIGITS));
            }

            Out_& operator<< (int32_t i){
                char buffer[15];
                StringUtil::itoa(i,buffer,10);
                __semi_call(SYS_WRITE0, buffer);
                return *this;
            }

            Out_& operator<< (uint32_t i){
                char buffer[15];
                StringUtil::modp_uitoa10(i,buffer);
                __semi_call(SYS_WRITE0, buffer);
                return *this;
            }

            Out_& operator<< (int i){
                char buffer[15];
                StringUtil::itoa(i,buffer,10);
                __semi_call(SYS_WRITE0, buffer);
                return *this;
            }

            Out_& operator<< (unsigned int i){
                char buffer[15];
                StringUtil::modp_uitoa10(i,buffer);
                __semi_call(SYS_WRITE0, buffer);
                return *this;
            }

            Out_& operator<< (int16_t i){
                return operator<<((int32_t) i);
            }

            Out_& operator<< (uint16_t i){
                return operator<<((uint32_t) i);
            }

            Out_& operator<< (int8_t i){
                return operator<<((int32_t) i);
            }

            Out_& operator<< (uint8_t i){
                return operator<<((uint32_t) i);
            }
        };

        static Out_ out;
};

}
#endif /* ! __ASSEMBLER__ */

#endif /* SEMIHOSTING_H_INCLUDED */
