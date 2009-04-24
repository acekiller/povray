/****************************************************************************
*                   parse.h
*
*  This header file is included by all all language parsing C modules in
*  POV-Ray.
*
*  from Persistence of Vision(tm) Ray Tracer
*  Copyright 1996 Persistence of Vision Team
*---------------------------------------------------------------------------
*  NOTICE: This source code file is provided so that users may experiment
*  with enhancements to POV-Ray and to port the software to platforms other
*  than those supported by the POV-Ray Team.  There are strict rules under
*  which you are permitted to use this file.  The rules are in the file
*  named POVLEGAL.DOC which should be distributed with this file. If
*  POVLEGAL.DOC is not available or for more info please contact the POV-Ray
*  Team Coordinator by leaving a message in CompuServe's Graphics Developer's
*  Forum.  The latest version of POV-Ray may be found there as well.
*
* This program is based on the popular DKB raytracer version 2.12.
* DKBTrace was originally written by David K. Buck.
* DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
*
*****************************************************************************/

#ifndef PARSE_H
#define PARSE_H


/*****************************************************************************
* Global preprocessor defines
******************************************************************************/

#define MAX_STRING_INDEX 127

/* Here we create our own little language for doing the parsing.  It
  makes the code easier to read. */

#define EXPECT { int Exit_Flag; Exit_Flag = FALSE; \
 while (!Exit_Flag) {Get_Token();  switch (Token.Token_Id) {
#define CASE(x) case x:
#define CASE2(x, y) case x: case y:
#define CASE3(x, y, z) case x: case y: case z:
#define CASE4(w, x, y, z) case w: case x: case y: case z:
#define CASE5(v, w, x, y, z) case v: case w: case x: case y: case z:
#define CASE6(u, v, w, x, y, z) case u: case v: case w: case x: case y: case z:
#define END_CASE break;
#define EXIT Exit_Flag = TRUE;
#define OTHERWISE default:
#define END_EXPECT } } }
#define GET(x) Get_Token(); if (Token.Token_Id != x) Parse_Error (x);
#define ALLOW(x) Get_Token(); if (Token.Token_Id != x) Unget_Token();
#define UNGET Unget_Token();
#define CASE_FLOAT CASE2 (LEFT_PAREN_TOKEN, FLOAT_FUNCT_TOKEN)\
 CASE2 (PLUS_TOKEN, DASH_TOKEN) UNGET
#define CASE_VECTOR CASE2 (VECTOR_FUNCT_TOKEN,LEFT_ANGLE_TOKEN) \
 CASE2 (U_TOKEN,V_TOKEN) CASE_FLOAT
#define CASE_EXPRESS CASE_VECTOR

#define CASE_COLOUR CASE3 (COLOUR_TOKEN,COLOUR_KEY_TOKEN,COLOUR_ID_TOKEN) UNGET

#define MAX_BRACES 200

/* Token Definitions for Parser */

#define ABS_TOKEN                         0
#define ATAN2_TOKEN                       1
#define CEIL_TOKEN                        2
#define CLOCK_TOKEN                       3
#define COS_TOKEN                         4
#define DEGREES_TOKEN                     5
#define DIV_TOKEN                         6
#define EXP_TOKEN                         7
#define FLOAT_ID_TOKEN                    8
#define FLOAT_TOKEN                       9
#define FLOOR_TOKEN                      10
#define INT_TOKEN                        11
#define LOG_TOKEN                        12
#define MAX_TOKEN                        13
#define MIN_TOKEN                        14
#define MOD_TOKEN                        15
#define PI_TOKEN                         16
#define POW_TOKEN                        17
#define RADIANS_TOKEN                    18
#define SIN_TOKEN                        19
#define SQRT_TOKEN                       20
#define VDOT_TOKEN                       21
#define VLENGTH_TOKEN                    22
#define VERSION_TOKEN                    23
#define ON_TOKEN                         24
#define OFF_TOKEN                        25
#define TRUE_TOKEN                       26
#define FALSE_TOKEN                      27
#define YES_TOKEN                        28
#define NO_TOKEN                         29
#define ACOS_TOKEN                       30
#define ASIN_TOKEN                       31
#define TAN_TOKEN                        32
#define ASC_TOKEN                        33
#define STRLEN_TOKEN                     34
#define VAL_TOKEN                        35
#define FILE_EXISTS_TOKEN                36
#define STRCMP_TOKEN                     37
#define SEED_TOKEN                       38
#define RAND_TOKEN                       39
#define FLOAT_FUNCT_TOKEN                40

/*WARNING: All new functions returning a float value must be placed
  before FLOAT_FUNCT_TOKEN or the parser won't work right. */

#define VAXIS_ROTATE_TOKEN               41
#define VCROSS_TOKEN                     42
#define VECTOR_ID_TOKEN                  43
#define VNORMALIZE_TOKEN                 44
#define VROTATE_TOKEN                    45
#define X_TOKEN                          46
#define Y_TOKEN                          47
#define Z_TOKEN                          48
#define VECTOR_FUNCT_TOKEN               49

/*WARNING: All new functions returning a vector value must be placed
  before VECTOR_FUNCT_TOKEN or the parser won't work right. */

#define ALPHA_TOKEN                      50
#define BLUE_TOKEN                       51
#define FILTER_TOKEN                     52
#define TRANSMIT_TOKEN                   53
#define GREEN_TOKEN                      54
#define RED_TOKEN                        55
#define RGBF_TOKEN                       56
#define RGBFT_TOKEN                      57
#define RGBT_TOKEN                       58
#define RGB_TOKEN                        59
#define COLOUR_KEY_TOKEN                 60

/*WARNING: All new keywords which may preceed a COLOUR must be placed
  before COLOUR_KEY_TOKEN or the parser won't work right. */

#define COLOR_TOKEN                      61
#define COLOUR_TOKEN                     62
#define COLOUR_ID_TOKEN                  63

/*Note:Not 100% sure about the above tokens.  May move them up later. */

#define ADAPTIVE_TOKEN                   64
#define AGATE_TOKEN                      65
#define ALL_TOKEN                        66
#define AMBIENT_TOKEN                    67
#define AMPERSAND_TOKEN                  68
#define AREA_LIGHT_TOKEN                 69
#define AT_TOKEN                         70
#define BACK_QUOTE_TOKEN                 71
#define BACK_SLASH_TOKEN                 72
#define BAR_TOKEN                        73
#define BICUBIC_PATCH_TOKEN              74
#define BLOB_TOKEN                       75
#define BOUNDED_BY_TOKEN                 76
#define BOX_TOKEN                        77
#define BOZO_TOKEN                       78
#define BRICK_TOKEN                      79
#define BRILLIANCE_TOKEN                 80
#define BUMPS_TOKEN                      81
#define BUMPY1_TOKEN                     82
#define BUMPY2_TOKEN                     83
#define BUMPY3_TOKEN                     84
#define BUMP_MAP_TOKEN                   85
#define BUMP_SIZE_TOKEN                  86
#define CAMERA_ID_TOKEN                  87
#define CAMERA_TOKEN                     88
#define CHECKER_TOKEN                    89
#define CLIPPED_BY_TOKEN                 90
#define COLON_TOKEN                      91
#define COLOR_MAP_TOKEN                  92
#define COLOUR_MAP_ID_TOKEN              93
#define COLOUR_MAP_TOKEN                 94
#define COMMA_TOKEN                      95
#define COMPONENT_TOKEN                  96
#define COMPOSITE_TOKEN                  97
#define CONE_TOKEN                       98
#define CRAND_TOKEN                      99
#define CUBIC_TOKEN                     100
#define CYLINDER_TOKEN                  101
#define DASH_TOKEN                      102
#define DECLARE_TOKEN                   103
#define DEFAULT_TOKEN                   104
#define DENTS_TOKEN                     105
#define DIFFERENCE_TOKEN                106
#define DIFFUSE_TOKEN                   107
#define DIRECTION_TOKEN                 108
#define DISC_TOKEN                      109
#define DISTANCE_TOKEN                  110
#define DOLLAR_TOKEN                    111
#define END_OF_FILE_TOKEN               112
#define EQUALS_TOKEN                    113
#define EXCLAMATION_TOKEN               114
#define FALLOFF_TOKEN                   115
#define FINISH_ID_TOKEN                 116
#define FINISH_TOKEN                    117
#define FLATNESS_TOKEN                  118
#define FOG_TOKEN                       119
#define FREQUENCY_TOKEN                 120
#define GIF_TOKEN                       121
#define GRADIENT_TOKEN                  122
#define GRANITE_TOKEN                   123
#define HASH_TOKEN                      124
#define HAT_TOKEN                       125
#define HEIGHT_FIELD_TOKEN              126
#define HEXAGON_TOKEN                   127
#define IDENTIFIER_TOKEN                128
#define IFF_TOKEN                       129
#define IMAGE_MAP_TOKEN                 130
#define INCLUDE_TOKEN                   131
#define INTERPOLATE_TOKEN               132
#define INTERSECTION_TOKEN              133
#define INVERSE_TOKEN                   134
#define IOR_TOKEN                       135
#define JITTER_TOKEN                    136
#define LAMBDA_TOKEN                    137
#define LEFT_ANGLE_TOKEN                138
#define LEFT_CURLY_TOKEN                139
#define LEFT_PAREN_TOKEN                140
#define LEFT_SQUARE_TOKEN               141
#define LEOPARD_TOKEN                   142
#define LIGHT_SOURCE_TOKEN              143
#define LOCATION_TOKEN                  144
#define LOOKS_LIKE_TOKEN                145
#define LOOK_AT_TOKEN                   146
#define MANDEL_TOKEN                    147
#define MAP_TYPE_TOKEN                  148
#define MARBLE_TOKEN                    149
#define MATERIAL_MAP_TOKEN              150
#define MAX_INTERSECTIONS               151
#define MAX_TRACE_LEVEL_TOKEN           152
#define MERGE_TOKEN                     153
#define METALLIC_TOKEN                  154
#define MORTAR_TOKEN                    155
#define NO_SHADOW_TOKEN                 156
#define OBJECT_ID_TOKEN                 157
#define OBJECT_TOKEN                    158
#define OCTAVES_TOKEN                   159
#define OMEGA_TOKEN                     160
#define ONCE_TOKEN                      161
#define ONION_TOKEN                     162
#define PATTERN1_TOKEN                  163
#define PATTERN2_TOKEN                  164
#define PATTERN3_TOKEN                  165
#define PERCENT_TOKEN                   166
#define PHASE_TOKEN                     167
#define PHONG_SIZE_TOKEN                168
#define PHONG_TOKEN                     169
#define PIGMENT_ID_TOKEN                170
#define PIGMENT_TOKEN                   171
#define PLANE_TOKEN                     172
#define PLUS_TOKEN                      173
#define POINT_AT_TOKEN                  174
#define POLY_TOKEN                      175
#define POT_TOKEN                       176
#define QUADRIC_TOKEN                   177
#define QUARTIC_TOKEN                   178
#define QUESTION_TOKEN                  179
#define QUICK_COLOR_TOKEN               180
#define QUICK_COLOUR_TOKEN              181
#define RADIAL_TOKEN                    182
#define RADIUS_TOKEN                    183
#define BRICK_SIZE_TOKEN                184
#define REFLECTION_TOKEN                185
#define REFRACTION_TOKEN                186
#define RIGHT_ANGLE_TOKEN               187
#define RIGHT_CURLY_TOKEN               188
#define RIGHT_PAREN_TOKEN               189
#define RIGHT_SQUARE_TOKEN              190
#define RIGHT_TOKEN                     191
#define RIPPLES_TOKEN                   192
#define ROTATE_TOKEN                    193
#define ROUGHNESS_TOKEN                 194
#define SCALE_TOKEN                     195
#define SEMI_COLON_TOKEN                196
#define SINGLE_QUOTE_TOKEN              197
#define SKY_TOKEN                       198
#define SLASH_TOKEN                     199
#define SMOOTH_TOKEN                    200
#define SMOOTH_TRIANGLE_TOKEN           201
#define SPECULAR_TOKEN                  202
#define SPHERE_TOKEN                    203
#define SPOTLIGHT_TOKEN                 204
#define SPOTTED_TOKEN                   205
#define STAR_TOKEN                      206
#define STRING_LITERAL_TOKEN            207
#define STURM_TOKEN                     208
#define TEXTURE_ID_TOKEN                209
#define TEXTURE_TOKEN                   210
#define TGA_TOKEN                       211
#define THRESHOLD_TOKEN                 212
#define TIGHTNESS_TOKEN                 213
#define TILDE_TOKEN                     214
#define TILE2_TOKEN                     215
#define TILES_TOKEN                     216
#define TNORMAL_ID_TOKEN                217
#define TNORMAL_TOKEN                   218
#define TORUS_TOKEN                     219
#define TRACK_TOKEN                     220
#define TRANSFORM_ID_TOKEN              221
#define TRANSFORM_TOKEN                 222
#define TRANSLATE_TOKEN                 223
#define TRIANGLE_TOKEN                  224
#define TURBULENCE_TOKEN                225
#define TYPE_TOKEN                      226
#define UNION_TOKEN                     227
#define UP_TOKEN                        228
#define USE_COLOR_TOKEN                 229
#define USE_COLOUR_TOKEN                230
#define USE_INDEX_TOKEN                 231
#define U_STEPS_TOKEN                   232
#define V_STEPS_TOKEN                   233
#define WATER_LEVEL_TOKEN               234
#define WAVES_TOKEN                     235
#define WOOD_TOKEN                      236
#define WRINKLES_TOKEN                  237
#define BACKGROUND_TOKEN                238
#define OPEN_TOKEN                      239
#define AGATE_TURB_TOKEN                240
#define IRID_TOKEN                      241
#define THICKNESS_TOKEN                 242
#define IRID_WAVELENGTH_TOKEN           243
#define CRACKLE_TOKEN                   244
#define ADC_BAILOUT_TOKEN               245
#define FILL_LIGHT_TOKEN                246
#define NUMBER_OF_WAVES_TOKEN           247
#define FOG_TYPE_TOKEN                  248
#define FOG_ALT_TOKEN                   249
#define FOG_OFFSET_TOKEN                250
#define TEXT_TOKEN                      251
#define TTF_TOKEN                       252
#define REL_GE_TOKEN                    253
#define REL_LE_TOKEN                    254
#define REL_NE_TOKEN                    255
#define APERTURE_TOKEN                  256
#define BLUR_SAMPLES_TOKEN              257
#define FOCAL_POINT_TOKEN               258
#define QUILTED_TOKEN                   259
#define CONTROL0_TOKEN                  260
#define CONTROL1_TOKEN                  261
#define RAINBOW_TOKEN                   262
#define SKYSPHERE_TOKEN                 263
#define ANGLE_TOKEN                     264
#define WIDTH_TOKEN                     265
#define ARC_ANGLE_TOKEN                 266
#define PERSPECTIVE_TOKEN               267
#define ORTHOGRAPHIC_TOKEN              268
#define FISHEYE_TOKEN                   269
#define ULTRA_WIDE_ANGLE_TOKEN          270
#define OMNIMAX_TOKEN                   271
#define PANORAMIC_TOKEN                 272
#define TEST_CAMERA_1_TOKEN             273
#define TEST_CAMERA_2_TOKEN             274
#define TEST_CAMERA_3_TOKEN             275
#define TEST_CAMERA_4_TOKEN             276
#define LATHE_TOKEN                     277
#define LINEAR_SPLINE_TOKEN             278
#define QUADRATIC_SPLINE_TOKEN          279
#define CUBIC_SPLINE_TOKEN              280
#define POLYGON_TOKEN                   281
#define PRISM_TOKEN                     282
#define LINEAR_SWEEP_TOKEN              283
#define CONIC_SWEEP_TOKEN               284
#define SOR_TOKEN                       285
#define SPIRAL1_TOKEN                   286
#define SPIRAL2_TOKEN                   287
#define STRENGTH_TOKEN                  288
#define HIERARCHY_TOKEN                 289
#define RAMP_WAVE_TOKEN                 290
#define TRIANGLE_WAVE_TOKEN             291
#define SINE_WAVE_TOKEN                 292
#define SCALLOP_WAVE_TOKEN              293
#define PIGMENT_MAP_TOKEN               294
#define NORMAL_MAP_TOKEN                295
#define SLOPE_MAP_TOKEN                 296
#define TEXTURE_MAP_TOKEN               297
#define PIGMENT_MAP_ID_TOKEN            298
#define NORMAL_MAP_ID_TOKEN             299
#define SLOPE_MAP_ID_TOKEN              300
#define TEXTURE_MAP_ID_TOKEN            301
#define SUPERELLIPSOID_TOKEN            302
#define QUATERNION_TOKEN                303
#define CUBE_TOKEN                      304
#define MAX_ITERATION_TOKEN             305
#define HYPERCOMPLEX_TOKEN              306
#define RAINBOW_ID_TOKEN                307
#define FOG_ID_TOKEN                    308
#define SKYSPHERE_ID_TOKEN              309
#define ATMOSPHERE_ID_TOKEN             310
#define ATMOSPHERE_TOKEN                311
#define SCATTERING_TOKEN                312
#define AA_LEVEL_TOKEN                  313
#define AA_THRESHOLD_TOKEN              314
#define SAMPLES_TOKEN                   315
#define FADE_POWER_TOKEN                316
#define FADE_DISTANCE_TOKEN             317
#define TURB_DEPTH_TOKEN                318
#define PPM_TOKEN                       319
#define PGM_TOKEN                       320
#define INCIDENCE_TOKEN                 321
#define AVERAGE_TOKEN                   322
#define MESH_TOKEN                      323
#define WARP_TOKEN                      324
#define OFFSET_TOKEN                    325
#define REPEAT_TOKEN                    326
#define BLACK_HOLE_TOKEN                327
#define SPIRAL_TOKEN                    328
#define FLIP_TOKEN                      329
#define AMBIENT_LIGHT_TOKEN             330
#define IF_TOKEN                        331
#define ELSE_TOKEN                      332
#define END_TOKEN                       333
#define SWITCH_TOKEN                    334
#define CASE_TOKEN                      335
#define RANGE_TOKEN                     336
#define WHILE_TOKEN                     337
#define BREAK_TOKEN                     338
#define FALLOFF_ANGLE_TOKEN             339
#define CAUSTICS_TOKEN                  340
#define JULIA_FRACTAL_TOKEN             341
#define SQR_TOKEN                       342
#define RECIPROCAL_TOKEN                343
#define STR_TOKEN                       344
#define CONCAT_TOKEN                    345
#define CHR_TOKEN                       346
#define SUBSTR_TOKEN                    347
#define STRING_ID_TOKEN                 348
#define WARNING_TOKEN                   349
#define ERROR_TOKEN                     350
#define RENDER_TOKEN                    351
#define STATISTICS_TOKEN                352
#define DEBUG_TOKEN                     353
#define VARIANCE_TOKEN                  354
#define CONFIDENCE_TOKEN                355
#define RADIOSITY_TOKEN                 356
#define BRIGHTNESS_TOKEN                357
#define COUNT_TOKEN                     358
#define DISTANCE_MAXIMUM_TOKEN          359
#define ERROR_BOUND_TOKEN               360
#define GRAY_THRESHOLD_TOKEN            361
#define LOW_ERROR_FACTOR_TOKEN          362
#define MINIMUM_REUSE_TOKEN             363
#define NEAREST_COUNT_TOKEN             364
#define RECURSION_LIMIT_TOKEN           365
#define HF_GRAY_16_TOKEN                366
#define GLOBAL_SETTINGS_TOKEN           367
#define ECCENTRICITY_TOKEN              368
#define HOLLOW_TOKEN                    369
#define ATMOSPHERIC_ATTENUATION_TOKEN   370
#define MATRIX_TOKEN                    371
#define HALO_TOKEN                      372
#define HALO_ID_TOKEN                   373
#define CONSTANT_TOKEN                  374
#define LINEAR_TOKEN                    375
#define VOLUME_OBJECT_TOKEN             376
#define MAX_VALUE_TOKEN                 377
#define EXPONENT_TOKEN                  378
#define EMITTING_TOKEN                  379
#define ATTENUATING_TOKEN               380
#define GLOWING_TOKEN                   381
#define DUST_TOKEN                      382
#define DUST_TYPE_TOKEN                 383
#define VOLUME_RENDERED_TOKEN           384
#define VOL_WITH_LIGHT_TOKEN            385
#define PLANAR_MAPPING_TOKEN            386
#define SPHERICAL_MAPPING_TOKEN         387
#define CYLINDRICAL_MAPPING_TOKEN       388
#define BOX_MAPPING_TOKEN               389
#define PERIOD_TOKEN                    390
#define SYS_TOKEN                       391
#define STRUPR_TOKEN                    392
#define STRLWR_TOKEN                    393
#define U_TOKEN                         394
#define V_TOKEN                         395
#define IFDEF_TOKEN                     396
#define PNG_TOKEN                       397
#define PRECISION_TOKEN                 398
#define SLICE_TOKEN                     399
#define ASSUMED_GAMMA_TOKEN             400
#define ATAN_TOKEN                      401     /* fractals */
#define COSH_TOKEN                      402     /* fractals */
#define SINH_TOKEN                      403     /* fractals */
#define TANH_TOKEN                      404     /* fractals */
#define ATANH_TOKEN                     405     /* fractals */
#define ACOSH_TOKEN                     406     /* fractals */
#define ASINH_TOKEN                     407     /* fractals */
#define PWR_TOKEN                       408     /* fractals */
#define T_TOKEN                         409
#define IFNDEF_TOKEN                    410
#define LAST_TOKEN                      411


/*****************************************************************************
* Global typedefs
******************************************************************************/



/*****************************************************************************
* Global variables
******************************************************************************/

extern short Not_In_Default;
extern short Ok_To_Declare;


/*****************************************************************************
* Global functions
******************************************************************************/

void Parse_Error PARAMS((TOKEN Token_Id));
void Parse_Error_Str PARAMS((char *str));
void Parse_Begin PARAMS((void));
void Parse_End PARAMS((void));
void Parse_Comma PARAMS((void));
void Destroy_Frame PARAMS((void));
void Parse PARAMS((void));
void Write_Token PARAMS((TOKEN Token_Id, DATA_FILE *Data_File));
void MAError PARAMS((char *str, size_t size));
void Warn_State PARAMS((TOKEN Token_Id, TOKEN Type));
void Warn PARAMS((DBL Level, char *str));
void Only_In PARAMS((char *s1,char *s2));
void Not_With PARAMS((char *s1,char *s2));
void Link_Textures PARAMS((TEXTURE **Old_Texture, TEXTURE *New_Texture));

void Parse_Object_Mods PARAMS((OBJECT *Object));
OBJECT *Parse_Object PARAMS((void));
void Parse_Default PARAMS((void));
void Parse_Declare PARAMS((void));
void Parse_Matrix PARAMS((MATRIX Matrix));


#endif
  
  
