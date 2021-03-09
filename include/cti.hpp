
#ifndef ARCTIC_CTI_HPP
#define ARCTIC_CTI_HPP

#include "ccd.hpp"
#include "roe.hpp"
#include "traps.hpp"

std::valarray<std::valarray<double>> clock_charge_in_one_direction(
    std::valarray<std::valarray<double>>& image_in, ROE roe, CCD ccd,
    std::valarray<Trap> traps, int express = 0, int offset = 0, int row_start = 0,
    int row_stop = 0, int column_start = 0, int column_stop = 0);

std::valarray<std::valarray<double>> add_cti(
    std::valarray<std::valarray<double>>& image_in, ROE* parallel_roe = nullptr,
    CCD* parallel_ccd = nullptr, std::valarray<Trap>* parallel_traps = nullptr,
    int parallel_express = 0, int parallel_offset = 0, ROE* serial_roe = nullptr,
    CCD* serial_ccd = nullptr, std::valarray<Trap>* serial_traps = nullptr,
    int serial_express = 0, int serial_offset = 0);

std::valarray<std::valarray<double>> remove_cti(
    std::valarray<std::valarray<double>>& image_in, int iterations,
    ROE* parallel_roe = nullptr, CCD* parallel_ccd = nullptr,
    std::valarray<Trap>* parallel_traps = nullptr, int parallel_express = 0,
    int parallel_offset = 0, ROE* serial_roe = nullptr, CCD* serial_ccd = nullptr,
    std::valarray<Trap>* serial_traps = nullptr, int serial_express = 0,
    int serial_offset = 0);

#endif  // ARCTIC_CTI_HPP
