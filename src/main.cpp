
#include <stdio.h>
#include <valarray>

#include "cti.hpp"
#include "roe.hpp"
#include "trap_managers.hpp"
#include "traps.hpp"
#include "util.hpp"

/*
    ...
*/
int main() {
        
    // Test image
    std::valarray<std::valarray<double>> image_pre_cti, image_post_cti;
    image_pre_cti = load_image_from_txt((char*) "dev/hst_acs_jc0a01h8q_raw.txt");
    // print_array_2D(image_pre_cti);
    TrapInstantCapture trap(10.0, -1.0 / log(0.5));
    ROE roe(1.0, true, false, true);
    CCD ccd(1e4, 0.0, 1.0);
    int express = 5;
    
    image_post_cti = clock_charge_in_one_direction(
        image_pre_cti, roe, ccd, std::valarray<Trap>{trap}, express);
        
    // print_array_2D(image_post_cti);
    save_image_to_txt((char*) "dev/image_out.txt", image_post_cti);

    return 0;
}
