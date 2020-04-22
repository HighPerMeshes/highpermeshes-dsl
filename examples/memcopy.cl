#define TYPE int

__kernel void                                                                   
__attribute__((task))                                                           
memcopy (__global TYPE * restrict src, __global TYPE * restrict dst, int size)
{                                                                               
  for(int i = 0; i < size; i++) {                                              
    dst[i] = src[i];                                                            
  }                                                                             
}

