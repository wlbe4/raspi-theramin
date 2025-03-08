/*
 * tof1020.h
 *
 *  Created on: March 8, 2025
 *      Author: Lior Weintraub
 */

#ifndef TOF1020_CHAR_DRIVER_H_
#define TOF1020_CHAR_DRIVER_H_

#define TOF1020_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef TOF1020_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "tof1020: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

struct tof1020_dev
{
    struct cdev cdev;     /* Char device structure      */
};


#endif /* TOF1020_CHAR_DRIVER_H_ */
