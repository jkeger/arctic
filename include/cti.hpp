
#ifndef ARCTIC_CTI_HPP
#define ARCTIC_CTI_HPP

#include "ccd.hpp"
#include "roe.hpp"
#include "traps.hpp"

std::valarray<std::valarray<double>> clock_charge_in_one_direction(
    std::valarray<std::valarray<double>>& image_in, ROE* roe, CCD* ccd,
    std::valarray<TrapInstantCapture>* instant_capture_traps,
    std::valarray<TrapSlowCapture>* slow_capture_traps,
    std::valarray<TrapContinuum>* continuum_traps, int express = 0, int offset = 0,
    int row_start = 0, int row_stop = -1, int column_start = 0, int column_stop = -1);

std::valarray<std::valarray<double>> add_cti(
    std::valarray<std::valarray<double>>& image_in,
    // Parallel
    ROE* parallel_roe = nullptr, CCD* parallel_ccd = nullptr,
    std::valarray<TrapInstantCapture>* parallel_instant_capture_traps = nullptr,
    std::valarray<TrapSlowCapture>* parallel_slow_capture_traps = nullptr,
    std::valarray<TrapContinuum>* parallel_continuum_traps = nullptr,
    int parallel_express = 0, int parallel_offset = 0, int parallel_window_start = 0,
    int parallel_window_stop = -1,
    // Serial
    ROE* serial_roe = nullptr, CCD* serial_ccd = nullptr,
    std::valarray<TrapInstantCapture>* serial_instant_capture_traps = nullptr,
    std::valarray<TrapSlowCapture>* serial_slow_capture_traps = nullptr,
    std::valarray<TrapContinuum>* serial_continuum_traps = nullptr,
    int serial_express = 0, int serial_offset = 0, int serial_window_start = 0,
    int serial_window_stop = -1);

std::valarray<std::valarray<double>> remove_cti(
    std::valarray<std::valarray<double>>& image_in, int n_iterations,
    // Parallel
    ROE* parallel_roe = nullptr, CCD* parallel_ccd = nullptr,
    std::valarray<TrapInstantCapture>* parallel_instant_capture_traps = nullptr,
    std::valarray<TrapSlowCapture>* parallel_slow_capture_traps = nullptr,
    std::valarray<TrapContinuum>* parallel_continuum_traps = nullptr,
    int parallel_express = 0, int parallel_offset = 0, int parallel_window_start = 0,
    int parallel_window_stop = -1,
    // Serial
    ROE* serial_roe = nullptr, CCD* serial_ccd = nullptr,
    std::valarray<TrapInstantCapture>* serial_instant_capture_traps = nullptr,
    std::valarray<TrapSlowCapture>* serial_slow_capture_traps = nullptr,
    std::valarray<TrapContinuum>* serial_continuum_traps = nullptr,
    int serial_express = 0, int serial_offset = 0, int serial_window_start = 0,
    int serial_window_stop = -1);

#endif  // ARCTIC_CTI_HPP
