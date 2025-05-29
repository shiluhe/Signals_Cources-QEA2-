/**
 * @file iir_filter.c
 * @author BusyBox (busybox177634@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-12-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "string.h"
#include "stdio.h"

#include "iir_filter/iir_filter.h"

void iir_filter_init(struct IIR_Handle* iir, double a0, double a1, double a2, double b0, double b1, double b2)
{
    memset(iir, 0, sizeof(struct IIR_Handle));
    iir->a[0] = a0;
    iir->a[1] = a1;
    iir->a[2] = a2;
    iir->b[0] = b0;
    iir->b[1] = b1;
    iir->b[2] = b2;
}

double iir_filter_process(struct IIR_Handle* iir, double value)
{
    iir->x[2] = iir->x[1];
    iir->x[1] = iir->x[0];
    iir->x[0] = value;

    iir->y[2] = iir->y[1];
    iir->y[1] = iir->y[0];
    iir->y[0] = (iir->b[0] * iir->x[0] + iir->b[1] * iir->x[1] + iir->b[2] * iir->x[2] - iir->a[1] * iir->y[1] - iir->a[2] * iir->y[2]) / iir->a[0];

    return iir->y[0];
}