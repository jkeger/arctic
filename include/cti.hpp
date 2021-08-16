
#ifndef ARCTIC_CTI_HPP
#define ARCTIC_CTI_HPP

#include "ccd.hpp"
#include "roe.hpp"
#include "traps.hpp"

std::valarray<std::valarray<double>> clock_charge_in_one_direction(
    std::valarray<std::valarray<double>>& image_in, ROE* roe, CCD* ccd,
    std::valarray<TrapInstantCapture>* traps_ic,
    std::valarray<TrapSlowCapture>* traps_sc,
    std::valarray<TrapInstantCaptureContinuum>* traps_ic_co,
    std::valarray<TrapSlowCaptureContinuum>* traps_sc_co, int express = 0,
    int offset = 0, int row_start = 0, int row_stop = -1, int column_start = 0,
    int column_stop = -1, int print_inputs = -1);

std::valarray<std::valarray<double>> add_cti(
    std::valarray<std::valarray<double>>& image_in,
    // Parallel
    ROE* parallel_roe = nullptr, CCD* parallel_ccd = nullptr,
    std::valarray<TrapInstantCapture>* parallel_traps_ic = nullptr,
    std::valarray<TrapSlowCapture>* parallel_traps_sc = nullptr,
    std::valarray<TrapInstantCaptureContinuum>* parallel_traps_ic_co = nullptr,
    std::valarray<TrapSlowCaptureContinuum>* parallel_traps_sc_co = nullptr,
    int parallel_express = 0, int parallel_offset = 0, int parallel_window_start = 0,
    int parallel_window_stop = -1,
    // Serial
    ROE* serial_roe = nullptr, CCD* serial_ccd = nullptr,
    std::valarray<TrapInstantCapture>* serial_traps_ic = nullptr,
    std::valarray<TrapSlowCapture>* serial_traps_sc = nullptr,
    std::valarray<TrapInstantCaptureContinuum>* serial_traps_ic_co = nullptr,
    std::valarray<TrapSlowCaptureContinuum>* serial_traps_sc_co = nullptr,
    int serial_express = 0, int serial_offset = 0, int serial_window_start = 0,
    int serial_window_stop = -1, int iteration = 0);

std::valarray<std::valarray<double>> remove_cti(
    std::valarray<std::valarray<double>>& image_in, int n_iterations,
    // Parallel
    ROE* parallel_roe = nullptr, CCD* parallel_ccd = nullptr,
    std::valarray<TrapInstantCapture>* parallel_traps_ic = nullptr,
    std::valarray<TrapSlowCapture>* parallel_traps_sc = nullptr,
    std::valarray<TrapInstantCaptureContinuum>* parallel_traps_ic_co = nullptr,
    std::valarray<TrapSlowCaptureContinuum>* parallel_traps_sc_co = nullptr,
    int parallel_express = 0, int parallel_offset = 0, int parallel_window_start = 0,
    int parallel_window_stop = -1,
    // Serial
    ROE* serial_roe = nullptr, CCD* serial_ccd = nullptr,
    std::valarray<TrapInstantCapture>* serial_traps_ic = nullptr,
    std::valarray<TrapSlowCapture>* serial_traps_sc = nullptr,
    std::valarray<TrapInstantCaptureContinuum>* serial_traps_ic_co = nullptr,
    std::valarray<TrapSlowCaptureContinuum>* serial_traps_sc_co = nullptr,
    int serial_express = 0, int serial_offset = 0, int serial_window_start = 0,
    int serial_window_stop = -1);

#endif  // ARCTIC_CTI_HPP
