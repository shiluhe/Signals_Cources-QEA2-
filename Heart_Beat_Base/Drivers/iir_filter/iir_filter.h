/**
 * @file iir_filter.h
 * @author BusyBox (busybox177634@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

struct IIR_Handle
{
    double a[3];
    double b[3];
    double x[3];
    double y[3];
};

void iir_filter_init(struct IIR_Handle* iir, double a0, double a1, double a2, double b0, double b1, double b2);
double iir_filter_process(struct IIR_Handle* iir, double value);
