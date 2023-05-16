#pragma once

struct ErfinvConstant {
  float a[4] = { 0.886226899, -1.645349621, 0.914624893, -0.140543331 };
  float b[4] = { -2.118377725, 1.442710462, -0.329097515, 0.012229801 };
  float c[4] = { -1.970840454, -1.624906493, 3.429567803, 1.641345311 };
  float d[2] = { 3.543889200, 1.637067800 };
};

const char* UNARY_KERNEL_TEMPLATE = R"METAL(
#include <metal_stdlib>
using namespace metal;

struct ErfinvConstant {{
  float a[4];
  float b[4];
  float c[4];
  float d[2];

}};

kernel void erfinv_mps_kernel( device {0} *output [[buffer(0)]],
                            device {1} *input [[buffer(1)]],
                            constant ErfinvConstant& uniforms [[buffer(2)]],
                            uint index [[thread_position_in_grid]]) {{

  constant const float *a = uniforms.a;
  constant const float *b = uniforms.b;
  constant const float *c = uniforms.c;
  constant const float *d = uniforms.d;
  float y = input[index];
  float x, z, num, dem; /*working variables */
  /* coefficients in rational expansion */

  float y_abs = abs(y);
  if(y_abs > 1.0f){{
    output[index] = NAN;
    return;
  }}
  if(y_abs == 1.0f){{
    output[index] = copysign(INFINITY, y);
    return;
  }}
  if(y_abs <= 0.7f) {{
    z = y * y;
    num = (((a[3]*z + a[2])*z + a[1])*z + a[0]);
    dem = ((((b[3]*z + b[2])*z + b[1])*z +b[0]) * z + 1.0f);
    x = y * num / dem;
  }}
  else{{
    z = sqrt(-1.0f*log((1.0-y_abs)/2.0));
    num = ((c[3]*z + c[2])*z + c[1]) * z + c[0];
    dem = (d[1]*z + d[0])*z + 1.0f;
    x = copysign(num, y) / dem;
  }}

  output[index] = x;
}})METAL";