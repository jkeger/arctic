
#ifndef ARCTIC_CTI_HPP
#define ARCTIC_CTI_HPP

#include "ccd.hpp"
#include "roe.hpp"
#include "traps.hpp"

std::valarray<std::valarray<double>> clock_charge_in_one_direction(
    std::valarray<std::valarray<double>>& image_in, ROE roe, CCD ccd,
    std::valarray<Trap> traps, int express = 0, int offset = 0, int row_start = 0,
    int row_stop = 0, int column_start = 0, int column_stop = 0);

#endif  // ARCTIC_CTI_HPP
