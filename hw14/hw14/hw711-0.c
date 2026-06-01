#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hx711.h"

int main(void) {
    stdio_init_all();

    while (!stdio_usb_connected()) { tight_loop_contents(); }

    hx711_init();

    int i = 0;          // was: init i = 0
    uint64_t last_t = 0;

    while (true) {
        char m[100];
        int v[1000];
        int r[1000];    // added: raw values
        int num = 0;
        uint64_t t[1000];
        int avg = 828000;
        scanf("%d", &num);

        for (i=0; i<num; i++){
            int val = hx711_read_raw();
            r[i] = val;             // save raw before IIR
            avg = val*.1+avg*.9;
            v[i] = avg;
            t[i] = to_ms_since_boot(get_absolute_time());
        }
        for (i=0; i<num; i++){      // was: i++0
            printf("%d %llu %d %d\n", i, t[i], r[i], v[i]);
        }
    }
}
