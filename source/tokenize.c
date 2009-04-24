/****************************************************************************
*                tokenize.c
*
*  This module implements the first part of a two part parser for the scene
*  description files.  This phase changes the input file into tokens.
*
*  This module tokenizes the input file and sends the tokens created
*  to the parser (the second stage).  Tokens sent to the parser contain a
*  token ID, the line number of the token, and if necessary, some data for
*  the token.
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

#include <ctype.h>
#include "frame.h"
#include "povray.h"
#include "povproto.h"
#include "parse.h"
#include "povray.h"
#include "render.h"
#include "tokenize.h"
#include "express.h"



/*****************************************************************************
* Local preprocessor defines
******************************************************************************/



/*****************************************************************************
* Local typedefs
******************************************************************************/

#define MAX_INCLUDE_FILES 10

#define CALL(x) { if (!(x)) return (FALSE); }

#define COND_STACK_SIZE 200

typedef enum cond_type
{
  ROOT_COND=0,
  WHILE_COND,
  IF_TRUE_COND,
  IF_FALSE_COND,
  ELSE_COND,
  SWITCH_COND,
  CASE_TRUE_COND,
  CASE_FALSE_COND,
  SKIP_TIL_END_COND
} COND_TYPE;


typedef struct Reserved_Word_Struct RESERVED_WORD;

#define HASH_TABLE_SIZE 257

typedef struct Pov_Hash_Table_Struct HASH_TABLE;

struct Pov_Hash_Table_Struct
{
  RESERVED_WORD Entry;
  HASH_TABLE *next;
};




/*****************************************************************************
* Local variables
******************************************************************************/

/* Hash tables */

HASH_TABLE *Reserved_Words_Hash_Table[HASH_TABLE_SIZE];
HASH_TABLE *Symbol_Table_Hash_Table[HASH_TABLE_SIZE];

int Max_Symbols, Max_Constants;

static int String_Index;
char String[MAX_STRING_INDEX];
char String2[MAX_STRING_INDEX];

int Number_Of_Constants;

/* moved here to allow reinitialization */

static int token_count = 0, line_count = 10;

static int Number_Of_Symbols;
struct Constant_Struct *Constants;

static int Include_File_Index;
static DATA_FILE *Data_File;
static DATA_FILE Include_Files[MAX_INCLUDE_FILES];

struct Token_Struct Token;

static char **Echo_Buff;
static char  *Echo_Ptr;
static int    Echo_Indx;
static int    Echo_Line;
static int    Echo_Unget_Flag;
static int    Echo_Unget_Char;

typedef struct Cond_Stack_Entry CS_ENTRY;

struct Cond_Stack_Entry
{
  COND_TYPE Cond_Type;
  DBL Switch_Value;
  FILE *While_File;
  long While_Pos,While_Line_No;
};

static CS_ENTRY *Cond_Stack;
static int CS_Index, Skipping, Inside_Ifdef;

int input_file_in_memory = 0 ;

/*
 * Here are the reserved words.  If you need to add new words,
 * be sure to declare them in parse.h
 */

RESERVED_WORD Reserved_Words [LAST_TOKEN] = {
  {AA_LEVEL_TOKEN, "aa_level"},
  {AA_THRESHOLD_TOKEN, "aa_threshold"},
  {ABS_TOKEN, "abs"},
  {ACOS_TOKEN, "acos"},
  {ACOSH_TOKEN,"acosh"},
  {ADAPTIVE_TOKEN, "adaptive"},
  {ADC_BAILOUT_TOKEN, "adc_bailout"},
  {AGATE_TOKEN, "agate"},
  {AGATE_TURB_TOKEN, "agate_turb"},
  {ALL_TOKEN, "all"},
  {ALPHA_TOKEN, "alpha"},
  {AMBIENT_LIGHT_TOKEN, "ambient_light"},
  {AMBIENT_TOKEN, "ambient"},
  {AMPERSAND_TOKEN, "&"},
  {ANGLE_TOKEN, "angle"},
  {APERTURE_TOKEN, "aperture"},
  {ARC_ANGLE_TOKEN, "arc_angle"},
  {AREA_LIGHT_TOKEN, "area_light"},
  {ASC_TOKEN, "asc"},
  {ASIN_TOKEN, "asin"},
  {ASINH_TOKEN,"asinh"},
  {ASSUMED_GAMMA_TOKEN, "assumed_gamma"},
  {ATAN_TOKEN,"atan"},
  {ATAN2_TOKEN,"atan2"},
  {ATANH_TOKEN,"atanh"},
  {ATMOSPHERE_ID_TOKEN, "atmosphere identifier"},
  {ATMOSPHERE_TOKEN, "atmosphere"},
  {ATMOSPHERIC_ATTENUATION_TOKEN, "atmospheric_attenuation"},
  {ATTENUATING_TOKEN, "attenuating"},
  {AT_TOKEN, "@"},
  {AVERAGE_TOKEN, "average"},
  {BACKGROUND_TOKEN, "background"},
  {BACK_QUOTE_TOKEN, "`"},
  {BACK_SLASH_TOKEN, "\\"},
  {BAR_TOKEN, "|"},
  {BICUBIC_PATCH_TOKEN, "bicubic_patch"},
  {BLACK_HOLE_TOKEN, "black_hole"},
  {BLOB_TOKEN, "blob"},
  {BLUE_TOKEN, "blue"},
  {BLUR_SAMPLES_TOKEN, "blur_samples"},
  {BOUNDED_BY_TOKEN, "bounded_by"},
  {BOX_MAPPING_TOKEN, "box_mapping"},
  {BOX_TOKEN, "box"},
  {BOZO_TOKEN, "bozo"},
  {BREAK_TOKEN, "break"},
  {BRICK_SIZE_TOKEN, "brick_size"},
  {BRICK_TOKEN, "brick"},
  {BRIGHTNESS_TOKEN, "brightness" },
  {BRILLIANCE_TOKEN, "brilliance"},
  {BUMPS_TOKEN, "bumps"},
  {BUMPY1_TOKEN, "bumpy1"},
  {BUMPY2_TOKEN, "bumpy2"},
  {BUMPY3_TOKEN, "bumpy3"},
  {BUMP_MAP_TOKEN, "bump_map"},
  {BUMP_SIZE_TOKEN, "bump_size"},
  {CAMERA_ID_TOKEN, "camera identifier"},
  {CAMERA_TOKEN, "camera"},
  {CASE_TOKEN, "case"},
  {CAUSTICS_TOKEN, "caustics"},
  {CEIL_TOKEN, "ceil"},
  {CHECKER_TOKEN, "checker"},
  {CHR_TOKEN, "chr"},
  {CLIPPED_BY_TOKEN, "clipped_by"},
  {CLOCK_TOKEN,"clock"},
  {COLON_TOKEN, ":"},
  {COLOUR_ID_TOKEN, "colour identifier"},
  {COLOUR_KEY_TOKEN,"color keyword"},
  {COLOUR_MAP_ID_TOKEN, "colour map identifier"},
  {COLOUR_MAP_TOKEN, "color_map"},
  {COLOUR_MAP_TOKEN, "colour_map"},
  {COLOUR_TOKEN, "color"},
  {COLOUR_TOKEN, "colour"},
  {COMMA_TOKEN, ","},
  {COMPONENT_TOKEN, "component"},
  {COMPOSITE_TOKEN, "composite"},
  {CONCAT_TOKEN, "concat"},
  {CONE_TOKEN, "cone"},
  {CONFIDENCE_TOKEN, "confidence"},
  {CONIC_SWEEP_TOKEN, "conic_sweep"},
  {CONSTANT_TOKEN,"constant"},
  {CONTROL0_TOKEN,"control0"},
  {CONTROL1_TOKEN,"control1"},
  {COS_TOKEN,"cos"},
  {COSH_TOKEN,"cosh"},
  {COUNT_TOKEN, "count" },
  {CRACKLE_TOKEN, "crackle"},
  {CRAND_TOKEN, "crand"},
  {CUBE_TOKEN, "cube"},
  {CUBIC_SPLINE_TOKEN, "cubic_spline"},
  {CUBIC_TOKEN, "cubic"},
  {CYLINDER_TOKEN, "cylinder"},
  {CYLINDRICAL_MAPPING_TOKEN, "cylindrical_mapping"},
  {DASH_TOKEN, "-"},
  {DEBUG_TOKEN, "debug"},
  {DECLARE_TOKEN, "declare"},
  {DEFAULT_TOKEN, "default"},
  {DEGREES_TOKEN, "degrees"},
  {DENTS_TOKEN, "dents"},
  {DIFFERENCE_TOKEN, "difference"},
  {DIFFUSE_TOKEN, "diffuse"},
  {DIRECTION_TOKEN, "direction"},
  {DISC_TOKEN, "disc"},
  {DISTANCE_MAXIMUM_TOKEN, "distance_maximum" },
  {DISTANCE_TOKEN, "distance"},
  {DIV_TOKEN, "div"},
  {DOLLAR_TOKEN, "$"},
  {DUST_TOKEN, "dust"},
  {DUST_TYPE_TOKEN, "dust_type"},
  {ELSE_TOKEN, "else"},
  {EMITTING_TOKEN, "emitting"},
  {END_OF_FILE_TOKEN, "End of File"},
  {END_TOKEN, "end"},
  {EQUALS_TOKEN, "="},
  {ERROR_BOUND_TOKEN, "error_bound" },
  {ERROR_TOKEN, "error"},
  {ECCENTRICITY_TOKEN, "eccentricity"},
  {EXCLAMATION_TOKEN, "!"},
  {EXPONENT_TOKEN, "exponent"},
  {EXP_TOKEN,"exp"},
  {FADE_DISTANCE_TOKEN, "fade_distance"},
  {FADE_POWER_TOKEN, "fade_power"},
  {FALLOFF_ANGLE_TOKEN, "falloff_angle"},
  {FALLOFF_TOKEN, "falloff"},
  {FALSE_TOKEN, "false"},
  {FILE_EXISTS_TOKEN, "file_exists"},
  {FILL_LIGHT_TOKEN, "shadowless"},
  {FILTER_TOKEN, "filter"},
  {FINISH_ID_TOKEN, "finish identifier"},
  {FINISH_TOKEN, "finish"},
  {FISHEYE_TOKEN, "fisheye"},
  {FLATNESS_TOKEN, "flatness"},
  {FLIP_TOKEN, "flip"},
  {FLOAT_FUNCT_TOKEN,"float function"},
  {FLOAT_ID_TOKEN, "float identifier"},
  {FLOAT_TOKEN, "float constant"},
  {FLOOR_TOKEN, "floor"},
  {FOCAL_POINT_TOKEN, "focal_point"},
  {FOG_ALT_TOKEN, "fog_alt"},
  {FOG_ID_TOKEN, "fog identifier"},
  {FOG_OFFSET_TOKEN, "fog_offset"},
  {FOG_TOKEN, "fog"},
  {FOG_TYPE_TOKEN, "fog_type"},
  {FREQUENCY_TOKEN, "frequency"},
  {GIF_TOKEN, "gif"},
  {GLOBAL_SETTINGS_TOKEN, "global_settings" },
  {GLOWING_TOKEN, "glowing"},
  {GRADIENT_TOKEN, "gradient"},
  {GRANITE_TOKEN, "granite"},
  {GRAY_THRESHOLD_TOKEN, "gray_threshold" },
  {GREEN_TOKEN, "green"},
  {HALO_ID_TOKEN, "halo identifier"},
  {HALO_TOKEN, "halo"},
  {HASH_TOKEN, "#"},
  {HAT_TOKEN, "^"},
  {HEIGHT_FIELD_TOKEN, "height_field"},
  {HEXAGON_TOKEN, "hexagon"},
  {HF_GRAY_16_TOKEN, "hf_gray_16" },
  {HIERARCHY_TOKEN, "hierarchy"},
  {HOLLOW_TOKEN, "hollow"},
  {HYPERCOMPLEX_TOKEN, "hypercomplex"},
  {IDENTIFIER_TOKEN, "undeclared identifier"},
  {IFDEF_TOKEN, "ifdef"},
  {IFNDEF_TOKEN, "ifndef"},
  {IFF_TOKEN, "iff"},
  {IF_TOKEN, "if"},
  {IMAGE_MAP_TOKEN, "image_map"},
  {INCIDENCE_TOKEN, "incidence"},
  {INCLUDE_TOKEN, "include"},
  {INTERPOLATE_TOKEN, "interpolate"},
  {INTERSECTION_TOKEN, "intersection"},
  {INT_TOKEN,"int"},
  {INVERSE_TOKEN, "inverse"},
  {IOR_TOKEN, "ior"},
  {IRID_TOKEN, "irid"},
  {IRID_WAVELENGTH_TOKEN,"irid_wavelength"},
  {JITTER_TOKEN, "jitter"},
  {JULIA_FRACTAL_TOKEN, "julia_fractal"},
  {LAMBDA_TOKEN, "lambda"},
  {LATHE_TOKEN, "lathe"},
  {LEFT_ANGLE_TOKEN, "<"},
  {LEFT_CURLY_TOKEN, "{"},
  {LEFT_PAREN_TOKEN, "("},
  {LEFT_SQUARE_TOKEN, "["},
  {LEOPARD_TOKEN, "leopard"},
  {LIGHT_SOURCE_TOKEN, "light_source"},
  {LINEAR_SPLINE_TOKEN, "linear_spline"},
  {LINEAR_SWEEP_TOKEN, "linear_sweep"},
  {LINEAR_TOKEN, "linear"},
  {LOCATION_TOKEN, "location"},
  {LOG_TOKEN,"log"},
  {LOOKS_LIKE_TOKEN, "looks_like"},
  {LOOK_AT_TOKEN, "look_at"},
  {LOW_ERROR_FACTOR_TOKEN, "low_error_factor" },
  {MANDEL_TOKEN,"mandel"},
  {MAP_TYPE_TOKEN, "map_type"},
  {MARBLE_TOKEN, "marble"},
  {MATERIAL_MAP_TOKEN, "material_map"},
  {MATRIX_TOKEN, "matrix"},
  {MAX_INTERSECTIONS, "max_intersections"},
  {MAX_ITERATION_TOKEN, "max_iteration"},
  {MAX_TOKEN, "max"},
  {MAX_TRACE_LEVEL_TOKEN, "max_trace_level"},
  {MAX_VALUE_TOKEN, "max_value"},
  {MERGE_TOKEN,"merge"},
  {MESH_TOKEN, "mesh"},
  {METALLIC_TOKEN, "metallic"},
  {MINIMUM_REUSE_TOKEN, "minimum_reuse" },
  {MIN_TOKEN, "min"},
  {MOD_TOKEN,"mod"},
  {MORTAR_TOKEN, "mortar"},
  {NEAREST_COUNT_TOKEN, "nearest_count" },
  {NORMAL_MAP_ID_TOKEN,"normal_map identifier"},
  {NORMAL_MAP_TOKEN, "normal_map"},
  {NO_SHADOW_TOKEN, "no_shadow"},
  {NO_TOKEN, "no"},
  {NUMBER_OF_WAVES_TOKEN, "number_of_waves"},
  {OBJECT_ID_TOKEN, "object identifier"},
  {OBJECT_TOKEN, "object"},
  {OCTAVES_TOKEN, "octaves"},
  {OFFSET_TOKEN, "offset"},
  {OFF_TOKEN, "off"},
  {OMEGA_TOKEN, "omega"},
  {OMNIMAX_TOKEN, "omnimax"},
  {ONCE_TOKEN, "once"},
  {ONION_TOKEN, "onion"},
  {ON_TOKEN, "on"},
  {OPEN_TOKEN, "open"},
  {ORTHOGRAPHIC_TOKEN, "orthographic"},
  {PANORAMIC_TOKEN, "panoramic"},
  {PATTERN1_TOKEN, "pattern1"},
  {PATTERN2_TOKEN, "pattern2"},
  {PATTERN3_TOKEN, "pattern3"},
  {PERCENT_TOKEN, "%"},
  {PERIOD_TOKEN, ". (period)"},
  {PERSPECTIVE_TOKEN, "perspective"},
  {PGM_TOKEN, "pgm"},
  {PHASE_TOKEN, "phase"},
  {PHONG_SIZE_TOKEN, "phong_size"},
  {PHONG_TOKEN, "phong"},
  {PIGMENT_ID_TOKEN, "pigment identifier"},
  {PIGMENT_MAP_ID_TOKEN,"pigment_map identifier"},
  {PIGMENT_MAP_TOKEN, "pigment_map"},
  {PIGMENT_TOKEN, "pigment"},
  {PI_TOKEN,"pi"},
  {PLANAR_MAPPING_TOKEN, "planar_mapping"},
  {PLANE_TOKEN, "plane"},
  {PLUS_TOKEN, "+"},
  {PNG_TOKEN, "png"},
  {POINT_AT_TOKEN, "point_at"},
  {POLYGON_TOKEN, "polygon"},
  {POLY_TOKEN, "poly"},
  {POT_TOKEN, "pot"},
  {POW_TOKEN,"pow"},
  {PPM_TOKEN, "ppm"},
  {PRECISION_TOKEN, "precision"},
  {PRISM_TOKEN, "prism"},
  {PWR_TOKEN,"pwr"},
  {QUADRATIC_SPLINE_TOKEN, "quadratic_spline"},
  {QUADRIC_TOKEN, "quadric"},
  {QUARTIC_TOKEN, "quartic"},
  {QUATERNION_TOKEN, "quaternion"},
  {QUESTION_TOKEN, "?"},
  {QUICK_COLOUR_TOKEN,"quick_color"},
  {QUICK_COLOUR_TOKEN,"quick_colour"},
  {QUILTED_TOKEN,"quilted"},
  {RADIAL_TOKEN, "radial"},
  {RADIANS_TOKEN, "radians"},
  {RADIOSITY_TOKEN, "radiosity" },
  {RADIUS_TOKEN, "radius"},
  {RAINBOW_ID_TOKEN, "rainbow identifier"},
  {RAINBOW_TOKEN, "rainbow"},
  {RAMP_WAVE_TOKEN, "ramp_wave"},
  {RAND_TOKEN, "rand"},
  {RANGE_TOKEN, "range"},
  {RECIPROCAL_TOKEN, "reciprocal" },
  {RECURSION_LIMIT_TOKEN, "recursion_limit" },
  {RED_TOKEN, "red"},
  {REFLECTION_TOKEN, "reflection"},
  {REFRACTION_TOKEN, "refraction"},
  {REL_GE_TOKEN,">="},
  {REL_LE_TOKEN,"<="},
  {REL_NE_TOKEN,"!="},
  {RENDER_TOKEN, "render"},
  {REPEAT_TOKEN, "repeat"},
  {RGBFT_TOKEN, "rgbft"},
  {RGBF_TOKEN,"rgbf"},
  {RGBT_TOKEN, "rgbt"},
  {RGB_TOKEN,"rgb"},
  {RIGHT_ANGLE_TOKEN, ">"},
  {RIGHT_CURLY_TOKEN, "}"},
  {RIGHT_PAREN_TOKEN, ")"},
  {RIGHT_SQUARE_TOKEN, "]"},
  {RIGHT_TOKEN, "right"},
  {RIPPLES_TOKEN, "ripples"},
  {ROTATE_TOKEN, "rotate"},
  {ROUGHNESS_TOKEN, "roughness"},
  {SAMPLES_TOKEN, "samples"},
  {SCALE_TOKEN, "scale"},
  {SCALLOP_WAVE_TOKEN, "scallop_wave"},
  {SCATTERING_TOKEN, "scattering"},
  {SEED_TOKEN, "seed"},
  {SEMI_COLON_TOKEN, ";"},
  {SINE_WAVE_TOKEN, "sine_wave"},
  {SINGLE_QUOTE_TOKEN, "'"},
  {SIN_TOKEN,"sin"},
  {SINH_TOKEN,"sinh"},
  {SKYSPHERE_ID_TOKEN, "sky_sphere identifier"},
  {SKYSPHERE_TOKEN, "sky_sphere"},
  {SKY_TOKEN, "sky"},
  {SLASH_TOKEN, "/"},
  {SLICE_TOKEN, "slice"},
  {SLOPE_MAP_ID_TOKEN,"slope_map identifier"},
  {SLOPE_MAP_TOKEN, "slope_map"},
  {SMOOTH_TOKEN,"smooth"},
  {SMOOTH_TRIANGLE_TOKEN, "smooth_triangle"},
  {SOR_TOKEN, "sor"},
  {SPECULAR_TOKEN, "specular"},
  {SPHERE_TOKEN, "sphere"},
  {SPHERICAL_MAPPING_TOKEN, "spherical_mapping"},
  {SPIRAL1_TOKEN, "spiral1"},
  {SPIRAL2_TOKEN, "spiral2"},
  {SPIRAL_TOKEN, "spiral"},
  {SPOTLIGHT_TOKEN, "spotlight"},
  {SPOTTED_TOKEN, "spotted"},
  {SQR_TOKEN,"sqr"},
  {SQRT_TOKEN,"sqrt"},
  {STAR_TOKEN, "*"},
  {STATISTICS_TOKEN, "statistics"},
  {STRCMP_TOKEN, "strcmp"},
  {STRENGTH_TOKEN, "strength"},
  {STRING_ID_TOKEN, "string identifier"},
  {STRING_LITERAL_TOKEN, "string literal"},
  {STRLEN_TOKEN, "strlen"},
  {STRLWR_TOKEN, "strlwr"},
  {STRUPR_TOKEN, "strupr"},
  {STR_TOKEN, "str"},
  {STURM_TOKEN, "sturm"},
  {SUBSTR_TOKEN, "substr"},
  {SUPERELLIPSOID_TOKEN, "superellipsoid"},
  {SWITCH_TOKEN, "switch"},
  {SYS_TOKEN, "sys"},
  {T_TOKEN, "t"},
  {TAN_TOKEN, "tan"},
  {TANH_TOKEN,"tanh"},
  {TEST_CAMERA_1_TOKEN, "test_camera_1"},
  {TEST_CAMERA_2_TOKEN, "test_camera_2"},
  {TEST_CAMERA_3_TOKEN, "test_camera_3"},
  {TEST_CAMERA_4_TOKEN, "test_camera_4"},
  {TEXTURE_ID_TOKEN, "texture identifier"},
  {TEXTURE_MAP_ID_TOKEN,"texture_map identifier"},
  {TEXTURE_MAP_TOKEN, "texture_map"},
  {TEXTURE_TOKEN, "texture"},
  {TEXT_TOKEN, "text"},
  {TGA_TOKEN, "tga"},
  {THICKNESS_TOKEN, "thickness"},
  {THRESHOLD_TOKEN, "threshold"},
  {TIGHTNESS_TOKEN, "tightness"},
  {TILDE_TOKEN, "~"},
  {TILE2_TOKEN, "tile2"},
  {TILES_TOKEN, "tiles"},
  {TNORMAL_ID_TOKEN, "normal identifier"},
  {TNORMAL_TOKEN, "normal"},
  {TORUS_TOKEN, "torus"},
  {TRACK_TOKEN, "track"},
  {TRANSFORM_ID_TOKEN, "transform identifier"},
  {TRANSFORM_TOKEN, "transform"},
  {TRANSLATE_TOKEN, "translate"},
  {TRANSMIT_TOKEN, "transmit"},
  {TRIANGLE_TOKEN, "triangle"},
  {TRIANGLE_WAVE_TOKEN, "triangle_wave"},
  {TRUE_TOKEN, "true"},
  {TTF_TOKEN, "ttf"},
  {TURBULENCE_TOKEN, "turbulence"},
  {TURB_DEPTH_TOKEN, "turb_depth"},
  {TYPE_TOKEN, "type"},
  {ULTRA_WIDE_ANGLE_TOKEN, "ultra_wide_angle"},
  {UNION_TOKEN, "union"},
  {UP_TOKEN, "up"},
  {USE_COLOUR_TOKEN,"use_color"},
  {USE_COLOUR_TOKEN,"use_colour"},
  {USE_INDEX_TOKEN,"use_index"},
  {U_STEPS_TOKEN, "u_steps"},
  {U_TOKEN, "u"},
  {VAL_TOKEN, "val"},
  {VARIANCE_TOKEN, "variance"},
  {VAXIS_ROTATE_TOKEN,"vaxis_rotate"},
  {VCROSS_TOKEN,"vcross"},
  {VDOT_TOKEN,"vdot"},
  {VECTOR_FUNCT_TOKEN,"vector function"},
  {VECTOR_ID_TOKEN, "vector identifier"},
  {VERSION_TOKEN, "version"},
  {VLENGTH_TOKEN,"vlength"},
  {VNORMALIZE_TOKEN,"vnormalize"},
  {VOLUME_OBJECT_TOKEN, "volume_object"},
  {VOLUME_RENDERED_TOKEN, "volume_rendered"},
  {VOL_WITH_LIGHT_TOKEN, "vol_with_light"},
  {VROTATE_TOKEN,"vrotate"},
  {V_STEPS_TOKEN, "v_steps"},
  {V_TOKEN, "v"},
  {WARNING_TOKEN, "warning"},
  {WARP_TOKEN, "warp"},
  {WATER_LEVEL_TOKEN, "water_level"},
  {WAVES_TOKEN, "waves"},
  {WHILE_TOKEN, "while"},
  {WIDTH_TOKEN, "width"},
  {WOOD_TOKEN, "wood"},
  {WRINKLES_TOKEN, "wrinkles"},
  {X_TOKEN,"x"},
  {YES_TOKEN, "yes"},
  {Y_TOKEN,"y"},
  {Z_TOKEN,"z"}
};



/*****************************************************************************
* Static functions
******************************************************************************/

static int Echo_ungetc PARAMS((int c));
static int Echo_getc PARAMS((void));
static int Skip_Spaces PARAMS((void));
static int Parse_C_Comments PARAMS((void));
static void Begin_String PARAMS((void));
static void Stuff_Character PARAMS((int c));
static void End_String PARAMS((void));
static int Read_Float PARAMS((void));
static void Parse_String_Literal PARAMS((void));
static void Read_Symbol PARAMS((void));
static int Find_Reserved PARAMS((char *s, int hash_value));
static int Find_Symbol PARAMS((char *s, int hash_value));
static void Skip_Tokens PARAMS((COND_TYPE cond));

static int get_hash_value PARAMS((char *s));
static void init_reserved_words_hash_table PARAMS((void));




/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void Initialize_Tokenizer()
{
  int i;

  Stage = STAGE_TOKEN_INIT;

  pre_init_tokenizer ();

  if (opts.Options & FROM_STDIN)
  {
    Data_File->File = stdin;
  }
  else
  {
     if (input_file_in_memory)
     {
        /* Note platforms which use this feature will
         * trap fopen so the NULL is ok [C.Cason 7/3/96]
         */
        Data_File->File = fopen (NULL, "rt") ;
     }
     else
     {
        Data_File->File = Locate_File (opts.Input_File_Name, READ_FILE_STRING,".pov",".POV",TRUE);
     }
  }

  if (Data_File->File == NULL)
  {
    Error ("Cannot open input file.");
  }

  Data_File->Filename = POV_MALLOC(strlen(opts.Input_File_Name)+1, "filename");

  strcpy (Data_File->Filename, opts.Input_File_Name);

  Data_File->Line_Number = 0;

  /* Allocate constants table. */

  Constants = (struct Constant_Struct *)POV_MALLOC((Max_Constants+1) * sizeof (struct Constant_Struct), "constants table");

  /* Init echo buffer. */

  Echo_Buff = (char **)POV_MALLOC(sizeof(char *) * Num_Echo_Lines, "echo buffer");

  for (Echo_Line = 0; Echo_Line < Num_Echo_Lines; Echo_Line++)
  {
    Echo_Buff[Echo_Line] = (char *)POV_MALLOC((size_t)Echo_Line_Length+10, "echo buffer");

    Echo_Buff[Echo_Line][0]='\0';
  }

  Echo_Line = 0;
  Echo_Ptr = Echo_Buff[0];

  /* Init conditional stack. */

  Cond_Stack = (CS_ENTRY*)POV_MALLOC(sizeof(CS_ENTRY*) * COND_STACK_SIZE, "conditional stack");

  Cond_Stack[0].Cond_Type    = ROOT_COND;
  Cond_Stack[0].Switch_Value = 0.0;

  /* Init token hash tables. */

  init_reserved_words_hash_table();

  for (i = 0; i < HASH_TABLE_SIZE; i++)
  {
    Symbol_Table_Hash_Table[i] = NULL;
  }
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void pre_init_tokenizer ()
{
  Token.Token_Line_No = 0;
  Token.Token_String  = NULL;
  Token.Unget_Token   = FALSE;
  Token.End_Of_File   = FALSE;
  Token.Filename      = NULL;
  Token.Constant_Data = NULL;

  Constants = NULL;
  Data_File = NULL;

  Number_Of_Constants = 0;
  line_count = 10;
  token_count = 0;
  Include_File_Index = 0;
  Echo_Indx=0;
  Echo_Line=0;
  Echo_Ptr=NULL;
  Echo_Buff=NULL;
  Echo_Unget_Flag=FALSE;
  Echo_Unget_Char='\0';

  Number_Of_Symbols = 0;

  CS_Index            = 0;
  Skipping            = FALSE;
  Inside_Ifdef        = FALSE;
  Cond_Stack          = NULL;
  Data_File = &Include_Files[0];
  Data_File->Filename = NULL;
}


/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void Terminate_Tokenizer()
{
  int i;
  HASH_TABLE *p, *temp;

  for (i = 0 ; i < HASH_TABLE_SIZE; i++)
  {
    p = Symbol_Table_Hash_Table[i];

    while (p)
    {
      temp = p->next;

      POV_FREE(p->Entry.Token_Name);

      POV_FREE(p);

      p = temp;
    }

    Symbol_Table_Hash_Table[i] = NULL;
  }

  for (i = 0 ; i < HASH_TABLE_SIZE; i++)
  {
    p = Reserved_Words_Hash_Table[i];

    while (p)
    {
      temp = p->next;

      POV_FREE(p);

      p = temp;
    }

    Reserved_Words_Hash_Table[i] = NULL;
  }

  if (Data_File->Filename != NULL)
  {
    fclose (Data_File->File);

    POV_FREE (Data_File->Filename);

    Data_File->Filename = NULL;
  }

  if (Echo_Buff != NULL)
  {
    for (Echo_Line = 0; Echo_Line < Num_Echo_Lines; Echo_Line++)
    {
      if (Echo_Buff[Echo_Line]!=NULL)
      {
        POV_FREE (Echo_Buff[Echo_Line]);

        Echo_Buff[Echo_Line]=NULL;
      }
    }

    POV_FREE (Echo_Buff);

    Echo_Buff = NULL;
  }

  if (Cond_Stack!=NULL)
  {
    POV_FREE (Cond_Stack);

    Cond_Stack = NULL;
  }
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   The main tokenizing routine.  Set up the files and continue parsing
*   until the end of file
*
*   Read a token from the input file and store it in the Token variable.
*   If the token is an INCLUDE token, then set the include file name and
*   read another token.
*
*   This function performs most of the work involved in tokenizing.  It
*   reads the first character of the token and decides which function to
*   call to tokenize the rest.  For simple tokens, it simply writes them
*   out to the token buffer.
*
* CHANGES
*
******************************************************************************/

void Get_Token ()
{
  register int c,c2;

  if (Token.Unget_Token)
  {
    Token.Unget_Token = FALSE;

    return;
  }

  if (Token.End_Of_File)
  {
    return;
  }


  Token.Token_Id = END_OF_FILE_TOKEN;

  while (Token.Token_Id == END_OF_FILE_TOKEN)
  {
    Skip_Spaces();

    c = Echo_getc();

    if (c == EOF)
    {
      if (Include_File_Index == 0)
      {
        if (CS_Index !=0)
          Error("End of file reached but #end expected.");

        Token.Token_Id = END_OF_FILE_TOKEN;

        Token.End_Of_File = TRUE;

        Status_Info("\n");

        return;
      }

      fclose(Data_File->File); /* added to fix open file buildup JLN 12/91 */

      POV_FREE (Data_File->Filename);

      Data_File = &Include_Files[--Include_File_Index];

      continue;
    }

    String[0] = c; /* This isn't necessar but helps debugging */

    String[1] = '\0';

    String_Index = 0;

    switch (c)
    {
      case '\n':
        Data_File->Line_Number++;
        COOPERATE_0
        break;

      case '{' :
        Write_Token (LEFT_CURLY_TOKEN, Data_File);
        break;
  
      case '}' :
        Write_Token (RIGHT_CURLY_TOKEN, Data_File);
        break;
  
      case '@' :
        Write_Token (AT_TOKEN, Data_File);
        break;
  
      case '&' :
        Write_Token (AMPERSAND_TOKEN, Data_File);
        break;
  
      case '`' :
        Write_Token (BACK_QUOTE_TOKEN, Data_File);
        break;
  
      case '\\':
        Write_Token (BACK_SLASH_TOKEN, Data_File);
        break;

      case '|' :
        Write_Token (BAR_TOKEN, Data_File);
        break;
  
      case ':' :
        Write_Token (COLON_TOKEN, Data_File);
        break;
  
      case ',' :
        Write_Token (COMMA_TOKEN, Data_File);
        break;
  
      case '-' :
        Write_Token (DASH_TOKEN, Data_File);
        break;
  
      case '$' :
        Write_Token (DOLLAR_TOKEN, Data_File);
        break;
  
      case '=' :
        Write_Token (EQUALS_TOKEN, Data_File);
        break;
  
      case '!' :
        c2 = Echo_getc();
        if (c2 == (int)'=')
        {
          Write_Token (REL_NE_TOKEN, Data_File);
        }
        else
        {
          Echo_ungetc(c2);
          Write_Token (EXCLAMATION_TOKEN, Data_File);
        }
        break;
  
      case '#' : 
        Parse_Directive(TRUE);
        /* Write_Token (HASH_TOKEN, Data_File);*/
        break;
  
      case '^' :
        Write_Token (HAT_TOKEN, Data_File);
        break;
  
      case '<' :
        c2 = Echo_getc();
        if (c2 == (int)'=')
        {
          Write_Token (REL_LE_TOKEN, Data_File);
        }
        else
        {
          Echo_ungetc(c2);
          Write_Token (LEFT_ANGLE_TOKEN, Data_File);
        }
        break;
  
      case '(' :
        Write_Token (LEFT_PAREN_TOKEN, Data_File);
        break;
  
      case '[' :
        Write_Token (LEFT_SQUARE_TOKEN, Data_File);
        break;
  
      case '%' :
        Write_Token (PERCENT_TOKEN, Data_File);
        break;
  
      case '+' :
        Write_Token (PLUS_TOKEN, Data_File);
        break;
  
      case '?' :
        Write_Token (QUESTION_TOKEN, Data_File);
        break;
  
      case '>' :
        c2 = Echo_getc();
        if (c2 == (int)'=')
        {
          Write_Token (REL_GE_TOKEN, Data_File);
        }
        else
        {
          Echo_ungetc(c2);
          Write_Token (RIGHT_ANGLE_TOKEN, Data_File);
        }
        break;
  
      case ')' :
        Write_Token (RIGHT_PAREN_TOKEN, Data_File);
        break;
  
      case ']' :
        Write_Token (RIGHT_SQUARE_TOKEN, Data_File);
        break;
  
      case ';' : /* Parser doesn't use it, so let's ignore it */
        /* Write_Token (SEMI_COLON_TOKEN, Data_File); */
        break;
  
      case '\'':
        Write_Token (SINGLE_QUOTE_TOKEN, Data_File);
        break;
  
        /* enable C++ style commenting */
      case '/' :
        c2 = Echo_getc();
        if(c2 != (int) '/' && c2 != (int) '*')
        {
          Echo_ungetc(c2);
          Write_Token (SLASH_TOKEN, Data_File);
          break;
        }
        if(c2 == (int)'*')
        {
          Parse_C_Comments();
          break;
        }
        while((c2 != (int)'\n') && (c2 != (int)'\r'))
        {
          c2=Echo_getc();
          if(c2==EOF)
          {
            Echo_ungetc(c2);
            break;
          }
        }
        if (c2 =='\n')
        {
          Data_File->Line_Number++;
        }
        COOPERATE_0
        break;

      case '*' :
        Write_Token (STAR_TOKEN, Data_File);
        break;
  
      case '~' :
        Write_Token (TILDE_TOKEN, Data_File);
        break;
  
      case '"' :
        Parse_String_Literal ();
        break;
  
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '.':
        Echo_ungetc(c);
        if (Read_Float () != TRUE)
          return;
        break;
  
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'h':
      case 'i':
      case 'j':
      case 'k':
      case 'l':
      case 'm':
      case 'n':
      case 'o':
      case 'p':
      case 'q':
      case 'r':
      case 's':
      case 't':
      case 'u':
      case 'v':
      case 'w':
      case 'x':
      case 'y':
      case 'z':
  
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'G':
      case 'H':
      case 'I':
      case 'J':
      case 'K':
      case 'L':
      case 'M':
      case 'N':
      case 'O':
      case 'P':
      case 'Q':
      case 'R':
      case 'S':
      case 'T':
      case 'U':
      case 'V':
      case 'W':
      case 'X':
      case 'Y':
      case 'Z':
      case '_':
        Echo_ungetc(c);
        Read_Symbol ();
        break;
      case '\t':
      case '\r':
      case '\032':   /* Control Z - EOF on many systems */
      case '\0':
        break;
  
      default:
        Error("Illegal character in input file, value is %02x.\n", c);
        break;
    }
  }

  token_count++;

  if (token_count > 1000)
  {
    token_count = 0;

    COOPERATE_0

    Check_User_Abort(FALSE);

    Status_Info(".");

    line_count++;

    if (line_count > 78)
    {
      line_count = 0;

      Status_Info ("\n");
    }
  }
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Mark that the token has been put back into the input stream.  The next
*   call to Get_Token will return the last-read token instead of reading a
*   new one from the file.
*
* CHANGES
*
******************************************************************************/

void Unget_Token ()
{
  Token.Unget_Token = TRUE;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Skip over spaces in the input file.
*
* CHANGES
*
******************************************************************************/

static int Skip_Spaces ()
{
  register int c;

  while (TRUE)
  {
    c = Echo_getc();

    if (c == EOF)
    {
      return (FALSE);
    }

    if (!(isspace(c) || c == 0x0A))
    {
      break;
    }

    if (c == '\n')
    {
      Data_File->Line_Number++;

      COOPERATE_0
    }
  }

  Echo_ungetc(c);

  return (TRUE);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   C style comments with asterik and slash - CdW 8/91.
*
* CHANGES
*
******************************************************************************/

static int Parse_C_Comments()
{
  register int c, c2;
  int End_Of_Comment;

  End_Of_Comment = FALSE;

  while (!End_Of_Comment)
  {
    c = Echo_getc();

    if (c == EOF)
    {
      Error ("No */ closing comment found.");
    }

    if (c == (int) '\n')
    {
      Data_File->Line_Number++;

      COOPERATE_0
    }

    if (c == (int) '*')
    {
      c2 = Echo_getc();

      if (c2 != (int) '/')
      {
        Echo_ungetc(c2);
      }
      else
      {
        End_Of_Comment = TRUE;
      }
    }

    /* Check for and handle nested comments */

    if (c == (int) '/')
    {
      c2 = Echo_getc();

      if (c2 != (int) '*')
      {
        Echo_ungetc(c2);
      }
      else
      {
        Parse_C_Comments();
      }
    }
  }

  return (TRUE);
}



/* The following routines make it easier to handle strings.  They stuff
   characters into a string buffer one at a time making all the proper
   range checks.  Call Begin_String to start, Stuff_Character to put
   characters in, and End_String to finish.  The String variable contains
   the final string. */

/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void Begin_String()
{
  String_Index = 0;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void Stuff_Character(c)
int c;
{
  if (String_Index < MAX_STRING_INDEX)
  {
    String[String_Index++] = (char)c;

    if (String_Index >= MAX_STRING_INDEX)
    {
      Error ("String too long.");
    }
  }
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void End_String()
{
  Stuff_Character((int)'\0');
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Read a float from the input file and tokenize it as one token. The phase
*   variable is 0 for the first character, 1 for all subsequent characters
*   up to the decimal point, 2 for all characters after the decimal
*   point, 3 for the E+/- and 4 for the exponent.  This helps to insure
*   that the number is formatted properly. E format added 9/91 CEY
*
* CHANGES
*
******************************************************************************/

static int Read_Float()
{
  register int c, Finished, Phase;

  Finished = FALSE;

  Phase = 0;

  Begin_String();

  while (!Finished)
  {
    c = Echo_getc();

    if (c == EOF)
    {
      Error ("Unexpected end of file.");
    }

    switch (Phase)
    {
      case 0:

        Phase = 1;

        if (isdigit(c))
        {
          Stuff_Character(c);
        }
        else
        {
          if (c == '.')
          {
            c = Echo_getc();

            if (c == EOF)
            {
              Error ("Unexpected end of file");
            }

            if (isdigit(c))
            {
              Stuff_Character('0');
              Stuff_Character('.');
              Stuff_Character(c);

              Phase = 2;
            }
            else
            {
              Echo_ungetc(c);

              Write_Token (PERIOD_TOKEN, Data_File);

              return(TRUE);
            }
          }
          else
          {
            Error ("Error in decimal number");
          }
        }

        break;

      case 1:
        if (isdigit(c))
        {
          Stuff_Character(c);
        }
        else
        {
          if (c == (int) '.')
          {
            Stuff_Character(c); Phase = 2;
          }
          else
          {
            if ((c == 'e') || (c == 'E'))
            {
              Stuff_Character(c); Phase = 3;
            }
            else
            {
              Finished = TRUE;
            }
          }
        }

        break;

      case 2:

        if (isdigit(c))
        {
          Stuff_Character(c);
        }
        else
        {
          if ((c == 'e') || (c == 'E'))
          {
            Stuff_Character(c); Phase = 3;
          }
          else
          {
            Finished = TRUE;
          }
        }

        break;

      case 3:

        if (isdigit(c) || (c == '+') || (c == '-'))
        {
          Stuff_Character(c); Phase = 4;
        }
        else
        {
          Finished = TRUE;
        }

        break;

      case 4:

        if (isdigit(c))
        {
          Stuff_Character(c);
        }
        else
        {
          Finished = TRUE;
        }

        break;
    }
  }

  Echo_ungetc(c);

  End_String();

  Write_Token (FLOAT_TOKEN, Data_File);

  if (sscanf (String, DBL_FORMAT_STRING, &Token.Token_Float) == 0)
  {
    return (FALSE);
  }

  return (TRUE);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Parse a string from the input file into a token.
*
* CHANGES
*
******************************************************************************/

static void Parse_String_Literal()
{
  register int c;

  Begin_String();

  while (TRUE)
  {
    c = Echo_getc();

    if (c == EOF)
    {
      Error ("No end quote for string.");
    }

    if (c == '\\')
    {
      switch(c = Echo_getc())
      {
        case '\n':
        case '\r':

          Error("Unterminated string literal.");

          break;

        case '\"':

          c = 0x22;

          break;

        case EOF:

          Error ("No end quote for string.");

          break;

        case '\\' :
          c='\\';
          break;

        default:

          Stuff_Character ('\\');
      }

      Stuff_Character (c);
    }
    else
    {
      if (c != (int) '"')
      {
        Stuff_Character (c);
      }
      else
      {
        break;
      }
    }
  }

  End_String();

  Write_Token (STRING_LITERAL_TOKEN, Data_File);

  Token.Token_String = String;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
*   Read in a symbol from the input file. Check to see if it is a reserved
*   word. If it is, write out the appropriate token. Otherwise, write the
*   symbol out to the symbol table and write out an IDENTIFIER token. An
*   identifier token is a token whose token number is greater than the
*   highest reserved word.
*
* CHANGES
*
******************************************************************************/

static void Read_Symbol()
{
  register int c, Symbol_Id;
  register int hash_value;
  HASH_TABLE *New_Entry;

  Begin_String();

  while (TRUE)
  {
    c = Echo_getc();

    if (c == EOF)
    {
      Error ("Unexpected end of file.");
    }

    if (isalpha(c) || isdigit(c) || c == (int) '_')
    {
      Stuff_Character(c);
    }
    else
    {
      Echo_ungetc(c);

      break;
    }
  }

  End_String();

  if (Inside_Ifdef)
  {
    Token.Token_Id = IDENTIFIER_TOKEN;

    return;
  }

  /* If its a reserved keyword, write it and return */

  if ((Symbol_Id = Find_Reserved(String, get_hash_value(String))) != -1)
  {
    Write_Token (Symbol_Id, Data_File);
  }
  else
  {
    if ((Symbol_Id = Find_Symbol(String, get_hash_value(String))) == -1)
    {
      /* Here its an unknown symbol. */

      if (Skipping)
      {
        Write_Token(IDENTIFIER_TOKEN, Data_File);
        return;
      }
      else
      {
        Number_Of_Symbols++;

        /* Add it to the table. */

        New_Entry = (HASH_TABLE *)POV_MALLOC(sizeof(HASH_TABLE), "hash table entry");

        New_Entry->Entry.Token_Number = Symbol_Id = Number_Of_Symbols;

        New_Entry->Entry.Token_Name = POV_MALLOC(strlen(String)+1, "identifier");

        strcpy(New_Entry->Entry.Token_Name, String);

        hash_value = get_hash_value(String);

        New_Entry->next = Symbol_Table_Hash_Table[hash_value];

        Symbol_Table_Hash_Table[hash_value] = New_Entry;
      }
    }

    Write_Token (LAST_TOKEN + Symbol_Id, Data_File);
  }
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   int - Index of the token in the reserved words table or -1 if it isn't there.
*
* AUTHOR
*
* DESCRIPTION
*
*   Use a hash table look-up to find a keyword given in the variable String.
*
* CHANGES
*
******************************************************************************/

static int Find_Reserved(s, hash_value)
char *s;
int hash_value;
{
  HASH_TABLE *p;

  p = Reserved_Words_Hash_Table[hash_value];

  while (p)
  {
    if (strcmp(s, p->Entry.Token_Name) == 0)
    {
      return(p->Entry.Token_Number);
    }

    p = p->next;
  }

  return(-1);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   int - Symbol ID, -1 if not found.
*
* AUTHOR
*
* DESCRIPTION
*
*   Use a hash table look-up to find symbol given in string s.
*
* CHANGES
*
******************************************************************************/

static int Find_Symbol(s, hash_value)
char *s;
int hash_value;
{
  HASH_TABLE *p;

  p = Symbol_Table_Hash_Table[hash_value];

  while (p)
  {
    if (strcmp(s, p->Entry.Token_Name) == 0)
    {
      return(p->Entry.Token_Number);
    }

    p = p->next;
  }

  return(-1);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static int Echo_getc()
{
  register int c;

  if (Echo_Unget_Flag)
  {
    Echo_Unget_Flag = FALSE;

    return(Echo_Unget_Char);
  }

  Echo_Ptr[Echo_Indx++] = c = getc(Data_File->File);

  if ((Echo_Indx > Echo_Line_Length) || (c == '\n'))
  {
    Echo_Ptr[Echo_Indx] = '\0';

    Echo_Indx = 0;

    Echo_Line++;

    if (Echo_Line == Num_Echo_Lines)
      Echo_Line = 0;

    Echo_Ptr=Echo_Buff[Echo_Line];
  }

  return(c);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static int Echo_ungetc(int c)
{
  Echo_Unget_Flag = TRUE;

  return(Echo_Unget_Char = c);
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void Where_Error ()
{
  int i;

  /* Return if no filename is specified. [DB 8/94] */

  if (Token.Filename == NULL)
  {
    return;
  }

  strcpy (&(Echo_Ptr[Echo_Indx])," <----ERROR\n");

  for (i=0;i<Num_Echo_Lines;i++)
  {
    Echo_Line++;

    if (Echo_Line==Num_Echo_Lines)
    {
      Echo_Line=0;
    }

    Error_Line(Echo_Buff[Echo_Line]);
  }

  Error_Line("\n%s:%d: error: ", Token.Filename, Token.Token_Line_No+1);
}




/*****************************************************************************
*
* FUNCTION    Parse_Directive
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR      Chris Young
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void Parse_Directive(After_Hash)
int After_Hash;
{
  DBL Value, Value2;
  int Flag;
  char *ts;
  COND_TYPE Curr_Type = Cond_Stack[CS_Index].Cond_Type;
  
  if (!Ok_To_Declare)
  {
    if (After_Hash)
    {
       Token.Token_Id=HASH_TOKEN; 
    }
    Token.Unget_Token = FALSE;

    return;
  }

  EXPECT
    CASE(IFDEF_TOKEN)
      CS_Index++;

      if (Skipping)
      {
        Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
        Skip_Tokens(SKIP_TIL_END_COND);
      }
      else
      {
        GET(LEFT_PAREN_TOKEN)
        Inside_Ifdef=TRUE;
        Get_Token();
        strcpy(String2,String);
        Inside_Ifdef=FALSE;
        GET(RIGHT_PAREN_TOKEN)
        
        if (Find_Symbol(String2, get_hash_value(String2)) != -1)
        {
           Cond_Stack[CS_Index].Cond_Type=IF_TRUE_COND;
        }
        else
        {
           Cond_Stack[CS_Index].Cond_Type=IF_FALSE_COND;
           Skip_Tokens(IF_FALSE_COND);
        }
      }
      EXIT
    END_CASE
    
    CASE(IFNDEF_TOKEN)
      CS_Index++;

      if (Skipping)
      {
        Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
        Skip_Tokens(SKIP_TIL_END_COND);
      }
      else
      {
        GET(LEFT_PAREN_TOKEN)
        Inside_Ifdef=TRUE;
        Get_Token();
        strcpy(String2,String);
        Inside_Ifdef=FALSE;
        GET(RIGHT_PAREN_TOKEN)
        
        if (Find_Symbol(String2, get_hash_value(String2)) != -1)
        {
           Cond_Stack[CS_Index].Cond_Type=IF_FALSE_COND;
           Skip_Tokens(IF_FALSE_COND);
        }
        else
        {
           Cond_Stack[CS_Index].Cond_Type=IF_TRUE_COND;
        }
      }
      EXIT
    END_CASE
    
    CASE(IF_TOKEN)
      CS_Index++;

      if (Skipping)
      {
        Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
        Skip_Tokens(SKIP_TIL_END_COND);
      }
      else
      {
        Value=Parse_Float_Param();
      
        if (fabs(Value)>EPSILON)
        {
           Cond_Stack[CS_Index].Cond_Type=IF_TRUE_COND;
        }
        else
        {
           Cond_Stack[CS_Index].Cond_Type=IF_FALSE_COND;
           Skip_Tokens(IF_FALSE_COND);
        }
      }
      EXIT
    END_CASE


    CASE(WHILE_TOKEN)
      CS_Index++;

      if (Skipping)
      {
        Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
        Skip_Tokens(SKIP_TIL_END_COND);
      }
      else
      {
        Cond_Stack[CS_Index].While_File    = Data_File->File;
        Cond_Stack[CS_Index].While_Pos     = ftell(Data_File->File);
        Cond_Stack[CS_Index].While_Line_No = Data_File->Line_Number;

        if (Echo_Unget_Flag)
        {
           Cond_Stack[CS_Index].While_Pos--;
        }

        Value=Parse_Float_Param();
       
        if (fabs(Value)>EPSILON)
        {
           Cond_Stack[CS_Index].Cond_Type = WHILE_COND;
        }
        else
        {
           Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
           Skip_Tokens(SKIP_TIL_END_COND);
        }
      }
      EXIT
    END_CASE
    

    CASE(ELSE_TOKEN)
      switch (Curr_Type)
      {
         case IF_TRUE_COND:
           Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
           Skip_Tokens(SKIP_TIL_END_COND);
           break;
           
         case IF_FALSE_COND:
           Cond_Stack[CS_Index].Cond_Type = ELSE_COND;
           Token.Token_Id=HASH_TOKEN; /*insures Skip_Token takes notice*/
           UNGET
           break;
         
         case CASE_TRUE_COND:
         case SKIP_TIL_END_COND:
           break;

         case CASE_FALSE_COND:
           Cond_Stack[CS_Index].Cond_Type = CASE_TRUE_COND;
           if (Skipping)
           {
              Token.Token_Id=HASH_TOKEN; /*insures Skip_Token takes notice*/
              UNGET
           }
           break;

         default:
           Error("Mis-matched '#else'.");
      }
      EXIT
    END_CASE

    CASE(SWITCH_TOKEN)
      CS_Index++;

      if (Skipping)
      {
        Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
        Skip_Tokens(SKIP_TIL_END_COND);
      }
      else
      {
        Cond_Stack[CS_Index].Switch_Value=Parse_Float_Param();
        Cond_Stack[CS_Index].Cond_Type=SWITCH_COND;
        EXPECT
          CASE2(CASE_TOKEN,RANGE_TOKEN)
            if (Token.Token_Id==CASE_TOKEN)
            {
              Value=Parse_Float_Param();
              Flag = (fabs(Value-Cond_Stack[CS_Index].Switch_Value)<EPSILON);
            }
            else
            {
              Parse_Float_Param2(&Value,&Value2);
              Flag = ((Cond_Stack[CS_Index].Switch_Value >= Value) &&
                      (Cond_Stack[CS_Index].Switch_Value <= Value2));
            }
 
            if(Flag)
            {
              Cond_Stack[CS_Index].Cond_Type=CASE_TRUE_COND;
            }
            else
            {
              Cond_Stack[CS_Index].Cond_Type=CASE_FALSE_COND;
              Skip_Tokens(CASE_FALSE_COND);
            }
            EXIT
          END_CASE

          OTHERWISE
            Error("#switch not followed by #case or #range.");
          END_CASE
        END_EXPECT
      }
      EXIT
    END_CASE

    CASE(BREAK_TOKEN)
      if (Curr_Type==CASE_TRUE_COND)
      {
        Cond_Stack[CS_Index].Cond_Type=SKIP_TIL_END_COND;
        Skip_Tokens(SKIP_TIL_END_COND);
      }          
      EXIT
    END_CASE
    
    CASE2(CASE_TOKEN,RANGE_TOKEN)
      switch(Curr_Type)
      {
        case CASE_TRUE_COND:
        case CASE_FALSE_COND:
          if (Token.Token_Id==CASE_TOKEN)
          {
            Value=Parse_Float_Param();
            Flag = (fabs(Value-Cond_Stack[CS_Index].Switch_Value)<EPSILON);
          }
          else
          {
            Parse_Float_Param2(&Value,&Value2);
            Flag = ((Cond_Stack[CS_Index].Switch_Value >= Value) &&
                    (Cond_Stack[CS_Index].Switch_Value <= Value2));
          }
          
          if(Flag && (Curr_Type==CASE_FALSE_COND))
          {
            Cond_Stack[CS_Index].Cond_Type=CASE_TRUE_COND;
            if (Skipping)
            {
               Token.Token_Id=HASH_TOKEN; /*insures Skip_Token takes notice*/
               UNGET
            }
          }
          break;           

        case SWITCH_COND:
          UNGET
        case SKIP_TIL_END_COND:
          break;

        default:
          Error("Mis-matched '#case' or '#range'.");
      }
      EXIT
    END_CASE
    
    CASE(END_TOKEN)
      switch (Curr_Type)
      {
         case IF_FALSE_COND:
           Token.Token_Id=HASH_TOKEN; /*insures Skip_Token takes notice*/
           UNGET
         case IF_TRUE_COND:
         case ELSE_COND:
         case CASE_TRUE_COND:
         case CASE_FALSE_COND:
         case SKIP_TIL_END_COND:
           if (--CS_Index < 0)
           {
              Error("Mis-matched '#end'.");
           }
           if (Skipping)
           {
              Token.Token_Id=HASH_TOKEN; /*insures Skip_Token takes notice*/
              UNGET
           }
           EXIT
           break;
         
         case WHILE_COND:
           if (Cond_Stack[CS_Index].While_File != Data_File->File)
           { 
              Error("#while loop didn't end in file where it started.");
           }
           
           if (fseek(Data_File->File, Cond_Stack[CS_Index].While_Pos,0) < 0)
           {
              Error("Unable to seek in input file for #while directive.\n");
           }

           Data_File->Line_Number = Cond_Stack[CS_Index].While_Line_No;

           Value=Parse_Float_Param();
      
           if (fabs(Value)<EPSILON)
           {
             Cond_Stack[CS_Index].Cond_Type = SKIP_TIL_END_COND;
             Skip_Tokens(SKIP_TIL_END_COND);
           }
           break;

         default:
           Error("Mis-matched '#end'.");
      }
      EXIT
    END_CASE

    CASE (DECLARE_TOKEN)
      if (Skipping)
      {
         UNGET
         EXIT
      }
      else
      {
         Parse_Declare ();
         if (Token.Unget_Token && (Token.Token_Id==HASH_TOKEN))
         {
            Token.Unget_Token=FALSE;
         }
         else
         {
            EXIT
         }
      }
    END_CASE
    
    CASE (DEFAULT_TOKEN)
      Parse_Default();
      EXIT
    END_CASE

    CASE (INCLUDE_TOKEN)
      if (Skipping)
      {
         UNGET
      }
      else
      {
         Open_Include();
      }
      EXIT
    END_CASE

    CASE (FLOAT_FUNCT_TOKEN)
      if (Skipping)
      {
        UNGET
        EXIT
      }
      else
      {
        switch(Token.Function_Id)
        {
           case VERSION_TOKEN:
             Ok_To_Declare = FALSE;
             opts.Language_Version = Parse_Float ();
             Ok_To_Declare = TRUE;
             if (Token.Unget_Token && (Token.Token_Id==HASH_TOKEN))
             {
                Token.Unget_Token=FALSE;
             }
             else
             {
                EXIT
             }
             break;

           default:
             UNGET
             Parse_Error_Str ("object or directive.");
             break;
        }
      }
    END_CASE

    CASE(WARNING_TOKEN)
      if (Skipping)
      {
        UNGET
        EXIT
      }
      else
      {     
        ts=Parse_Formatted_String();
        Warning(0.0,ts);
        POV_FREE(ts);
      }
      EXIT
    END_CASE
      
    CASE(ERROR_TOKEN)
      if (Skipping)
      {
        UNGET
        EXIT
      }
      else
      {     
        ts=Parse_Formatted_String();
        POV_FREE(ts);
        Error("User error directive hit.");
      }
      EXIT
    END_CASE
      
    CASE(RENDER_TOKEN)
      if (Skipping)
      {
        UNGET
        EXIT
      }
      else
      {     
        ts=Parse_Formatted_String();
        Render_Info(ts);
        POV_FREE(ts);
      }
      EXIT
    END_CASE
      
    CASE(STATISTICS_TOKEN)
      if (Skipping)
      {
        UNGET
        EXIT
      }
      else
      {     
        ts=Parse_Formatted_String();
        Statistics(ts);
        POV_FREE(ts);
      }
      EXIT
    END_CASE
      
    CASE(DEBUG_TOKEN)
      if (Skipping)
      {
        UNGET
        EXIT
      }
      else
      {     
        ts=Parse_Formatted_String();
        Debug_Info(ts);
        POV_FREE(ts);
      }
      EXIT
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT
  
  if (Token.Unget_Token)
  {
    Token.Unget_Token = FALSE;
  }
  else
  {
    Token.Token_Id = END_OF_FILE_TOKEN;
  }
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

void Open_Include()
{
   char *temp;

   if (Skip_Spaces () != TRUE)
     Error ("Expecting a string after INCLUDE.\n");

   Include_File_Index++;

   if (Include_File_Index > MAX_INCLUDE_FILES)
     Error ("Too many nested include files.\n");

   temp = Parse_String();

   Data_File = &Include_Files[Include_File_Index];
   Data_File->Line_Number = 0;
   Data_File->Filename = temp;

   if ((Data_File->File = Locate_File (Data_File->Filename, READ_FILE_STRING,".inc",".INC",TRUE)) == NULL)
   {
      temp = Data_File->Filename;
      Data_File->Filename = NULL;  /* Keeps from closing failed file. */
      Stage=STAGE_INCLUDE_ERR;
      Error ("Cannot open include file %s.\n", temp);
   }

   Token.Token_Id = END_OF_FILE_TOKEN;
}



/*****************************************************************************
*
* FUNCTION
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
* DESCRIPTION
*
* CHANGES
*
******************************************************************************/

static void Skip_Tokens(cond)
COND_TYPE cond;
{
  int Temp      = CS_Index;
  int Prev_Skip = Skipping;

  Skipping=TRUE;

  while ((CS_Index > Temp) || ((CS_Index == Temp) && (Cond_Stack[CS_Index].Cond_Type == cond)))
  {
    Get_Token();
  }

  Skipping=Prev_Skip;

  if (Token.Token_Id==HASH_TOKEN)
  {
     Token.Token_Id=END_OF_FILE_TOKEN;
     Token.Unget_Token=FALSE;
  }
  else
  {
     UNGET
  }
}



/*****************************************************************************
*
* FUNCTION
*
*   get_hash_value
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Calculate hash value for a given string.
*
* CHANGES
*
*   Apr 1996 : Creation.
*
******************************************************************************/

static int get_hash_value(s)
char *s;
{
  unsigned int i = 0;

  while (*s)
  {
    i = (i << 1) ^ *s++;
  }

  return((int)(i % HASH_TABLE_SIZE));
}



/*****************************************************************************
*
* FUNCTION
*
*   init_reserved_words_hash_table
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   Sort token list.
*
* CHANGES
*
*   Apr 1996 : Creation.
*
******************************************************************************/

static void init_reserved_words_hash_table()
{
  int i;
  unsigned int hash_value;
  HASH_TABLE *New_Entry;

  for (i = 0; i < HASH_TABLE_SIZE; i++)
  {
    Reserved_Words_Hash_Table[i] = NULL;
  }

  for (i = 0; i < LAST_TOKEN; i++)
  {
    hash_value = get_hash_value(Reserved_Words[i].Token_Name);

    New_Entry = (HASH_TABLE *)POV_MALLOC(sizeof(HASH_TABLE), "hash table entry");

    New_Entry->Entry = Reserved_Words[i];

    New_Entry->next = Reserved_Words_Hash_Table[hash_value];

    Reserved_Words_Hash_Table[hash_value] = New_Entry;
  }
}

