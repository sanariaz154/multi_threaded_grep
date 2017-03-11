#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "args.h"

typedef struct part {
   char *start;
   size_t len;
} part_t;

/////////////////////////////////////////
static char *map = NULL;   //memory map 
static struct stat mapstat;

static pthread_mutex_t print_mutex;   //threads

void free_map(void)
{
   if (map) {
      munmap(map, mapstat.st_size);
      map = NULL;
   }
}


////////////////////////////////////
int init_map(const char *filename)
{
   int ret = 0;
   int fd = open(filename, O_RDONLY);

   if (fd < 0) {
      perror(filename);
      goto out;
   }

   if (fstat(fd, &mapstat) < 0) {
      perror(filename);
      goto out2;
   }

   map = mmap(NULL, mapstat.st_size, PROT_READ, MAP_SHARED, fd, 0);
      /* creates a new mapping in the virtual address space of the
       calling process.  The starting address for the new mapping is
       specified in addr.  The length argument specifies the length of the
       mapping.*/
   if (map == MAP_FAILED) {
      perror(filename);
      goto out2;
   }

   atexit(free_map);  //Free map automatically when program exits
   ret = 1;

 out2:

   close(fd);

 out:

   return ret;
}
////////////////////////////////////////////
void *run_part(void *data)
{
   char *line;
   char *next;
   size_t linelen;
   size_t offset;
   part_t *part = (part_t *)data;

  
   line = part->start;
   offset = 0;

   while (offset < part->len) {
      next = strchr(line, '\n');
      if(!next) {
         break;
      } 
      linelen = next - line + 1;        

      // Print line if it contains searchstring (needle)
      if (memmem(line, linelen - 1, opt.needle, opt.needlen)) {  //finds the start of the first occurrence of the substring needle 
                                                                    //length needlelen in the memory area haystack of length haystacklen.
         // and dont contain vstring (if present)
         if ((!opt.vstring) || (!memmem(line, linelen - 1, opt.vstring, opt.vlen))) {
            
            printf("%.*s", (int)linelen, line);
            // pthread_mutex_unlock(&print_mutex);
         }
      }

      offset += linelen;
      line = next + 1;
   }

   return NULL;
}

//////////////////////////////////////
#define MAX_partS 16

int main(int argc, char *argv[])
{
   char *next, *end;
   pthread_t threads[MAX_partS];
   part_t part[MAX_partS];
   int num_cpus, i;

   if (!parse_args(argc, argv)) {
      print_usage();
      return 1;
   }

   if (!init_map(opt.filename)) {
      return 1;
   }

   if (opt.single) {
      num_cpus = 1;
   } else {
      /* Keep num_cpus in range 1 - MAX_partS */
      num_cpus = (int)sysconf(_SC_NPROCESSORS_ONLN);
      if (num_cpus < 2) {
         num_cpus = 1;
      } else if (num_cpus > MAX_partS) {
         num_cpus = MAX_partS;
      }
   }

   /* Calculate start address and length of each part */
   part[0].start = map;
   part[0].len = (size_t)mapstat.st_size / num_cpus;
   for (i = 1; i < num_cpus; i++) {
           
           end = part[i - 1].start + part[i - 1].len;   
      if (end - map >= mapstat.st_size) {
         part[0].len = (size_t)mapstat.st_size;       // Past last line (few lines in file), set 1 part
         num_cpus = 1;
         break;
      }
      next = strchr(end, '\n');
      part[i - 1].len += next - end;
      part[i].start = next + 1;
      part[i].len = (size_t)mapstat.st_size / num_cpus;

      if (part[i].start - map >= mapstat.st_size) {
         /* Past last line */
         part[0].len = (size_t)mapstat.st_size;
         num_cpus = 1;
         break;
      }
   }

   /* Adjust last len with help of file size */
   if (num_cpus > 1) {
      size_t abs_start = part[i].start - map;
      part[i].len = mapstat.st_size - abs_start;
   }



  
   pthread_mutex_init(&print_mutex, NULL);

   /* Spawn num_cpus threads for one part each */
   for (i = 0; i < num_cpus; i++) {
      pthread_create(&threads[i], NULL, &run_part, (void *)&part[i]);
   }

 
   for (i = 0; i < num_cpus; i++) {
      pthread_join(threads[i], NULL);
   }


   pthread_mutex_destroy(&print_mutex);

   return 0;
}


