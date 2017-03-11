#ifndef __ARGS_H
#define __ARGS_H
struct opts
{
   /* Mandatory */
   char *filename;
   char *needle;
   int needlen;

   /* Optional */
   char *vstring;
   int vlen;
   int single;
   
};

/* Parsed options will be here */
extern struct opts opt;

extern int parse_args(int argc, char **argv);
extern void print_usage(void);


#endif 
