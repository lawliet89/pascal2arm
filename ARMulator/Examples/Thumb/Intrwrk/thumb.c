#include <stdio.h>

extern void arm_function(void);

int main(void)
{
    arm_function();
    printf("Hello from Thumb World\n");
}
