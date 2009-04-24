/****************************************************************************
*                parse.c
*
*  This module implements a parser for the scene description files.
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

#include "frame.h"
#include "vector.h"
#include "povproto.h"
#include "parse.h"
#include "parstxtr.h"
#include "atmosph.h"
#include "bezier.h"   
#include "blob.h"     
#include "boxes.h"
#include "colour.h"
#include "cones.h"    
#include "csg.h"      
#include "discs.h"
#include "express.h"  
#include "fractal.h"
#include "gif.h"      
#include "halos.h"
#include "hfield.h"
#include "iff.h"      
#include "image.h"    
#include "lathe.h"    
#include "polysolv.h"
#include "matrices.h"
#include "mesh.h"
#include "normal.h"
#include "objects.h"
#include "octree.h"
#include "pigment.h"
#include "planes.h"
#include "poly.h"
#include "polygon.h"
#include "povray.h"   
#include "pgm.h"      
#include "ppm.h"      
#include "prism.h"    
#include "quadrics.h" 
#include "radiosit.h"      
#include "render.h"   
#include "sor.h"      
#include "spheres.h"  
#include "super.h"
#include "targa.h"    
#include "texture.h"  
#include "tokenize.h" 
#include "torus.h"
#include "triangle.h" 
#include "truetype.h" 


/*****************************************************************************
* Local preprocessor defines
******************************************************************************/

/* Volume that is considered to be infinite. [DB 9/94] */

#define INFINITE_VOLUME BOUND_HUGE


/*****************************************************************************
* Local typedefs
******************************************************************************/



/*****************************************************************************
* Local variables
******************************************************************************/

short Have_Vector;
short Not_In_Default;
short Ok_To_Declare;

static TOKEN *Brace_Stack;
static int Brace_Index;
static CAMERA *Default_Camera;



/*****************************************************************************
* Static functions
******************************************************************************/

static TRANSFORM *Parse_Transform PARAMS((void));
static void Frame_Init PARAMS((void));
static void Parse_Coeffs PARAMS((int order, DBL *Coeffs));

static OBJECT *Parse_Bicubic_Patch PARAMS((void));
static OBJECT *Parse_Blob PARAMS((void));
static OBJECT *Parse_Bound_Clip PARAMS((void));
static OBJECT *Parse_Box PARAMS((void));
static OBJECT *Parse_Cone PARAMS((void));
static OBJECT *Parse_CSG PARAMS((int CSG_Type));
static OBJECT *Parse_Cylinder PARAMS((void));
static OBJECT *Parse_Disc PARAMS((void));
static OBJECT *Parse_Julia_Fractal PARAMS((void));
static OBJECT *Parse_HField PARAMS((void));
static OBJECT *Parse_Lathe PARAMS((void));
static OBJECT *Parse_Light_Source PARAMS((void));
static OBJECT *Parse_Object_Id PARAMS((void));
static OBJECT *Parse_Plane PARAMS((void));
static OBJECT *Parse_Poly PARAMS((int order));
static OBJECT *Parse_Polygon PARAMS((void));
static OBJECT *Parse_Prism PARAMS((void));
static OBJECT *Parse_Quadric PARAMS((void));
static OBJECT *Parse_Smooth_Triangle PARAMS((void));
static OBJECT *Parse_Sor PARAMS((void));
static OBJECT *Parse_Sphere PARAMS((void));
static OBJECT *Parse_Superellipsoid PARAMS((void));
static OBJECT *Parse_Torus PARAMS((void));
static OBJECT *Parse_Triangle PARAMS((void));
static OBJECT *Parse_Mesh PARAMS((void));
static TEXTURE *Parse_Mesh_Texture PARAMS((void));
static OBJECT *Parse_TrueType PARAMS((void));
static void Parse_Blob_Element_Mods PARAMS((BLOB_ELEMENT *Element));

static void Parse_Camera PARAMS((CAMERA **Camera_Ptr));
static void Parse_Frame PARAMS((void));

static char *Get_Token_String PARAMS((TOKEN Token_Id));
static int Test_Redefine PARAMS((int Previous, int a));
static void Destroy_Constants PARAMS((void));
static void Found_Instead PARAMS((void));
static void Link PARAMS((OBJECT *New_Object,OBJECT **Field,OBJECT **Old_Object_List));
static void Link_To_Frame PARAMS((OBJECT *Object));
static void Post_Process PARAMS((OBJECT *Object, OBJECT *Parent));
static void Parse_Global_Settings PARAMS((void));
static void Global_Setting_Warn PARAMS((void));

static void Set_CSG_Children_Hollow PARAMS((OBJECT *Object, int hollow));



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

/* Parse the file. */
void Parse ()
{
  Initialize_Tokenizer();
  Brace_Stack = (TOKEN *)POV_MALLOC(MAX_BRACES*sizeof (TOKEN), "brace stack");
  Brace_Index = 0;

  Default_Camera = Create_Camera();

  Default_Texture = Create_Texture();

  Default_Texture->Pigment = Create_Pigment();
  Default_Texture->Tnormal = NULL;
  Default_Texture->Finish  = Create_Finish();
  Default_Texture->Halo    = NULL;

  Not_In_Default = TRUE;
  Ok_To_Declare = TRUE;

  Frame_Init ();
  
  Stage = STAGE_PARSING;

  Parse_Frame ();

  if (Frame.Objects == NULL)
  {
    Error ("No objects in scene.");
  }
     
  Stage = STAGE_CLEANUP_PARSE;

  Terminate_Tokenizer();
  Destroy_Constants ();
  Destroy_Textures(Default_Texture); 
  Destroy_Camera(Default_Camera); 
  POV_FREE (Brace_Stack);

  Default_Texture = NULL;
  Default_Camera = NULL;
  Brace_Stack = NULL;
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

/* Set up the fields in the frame to default values. */
static
void Frame_Init ()
  {
   Frame.Camera = Copy_Camera(Default_Camera);
   Frame.Number_Of_Light_Sources = 0;  
   Frame.Light_Sources = NULL;
   Frame.Objects = NULL;
   Frame.Atmosphere_IOR = 1.0;
   Frame.Antialias_Threshold = opts.Antialias_Threshold;

/* dmf -- the first is physically "more correct".  The second works better */
/*   Make_Colour (Frame.Irid_Wavelengths, 0.70, 0.52, 0.48); */
   Make_Colour (Frame.Irid_Wavelengths, 0.25, 0.18, 0.14);
   Make_Colour (Frame.Background_Colour, 0.0, 0.0, 0.0);
   Make_Colour (Frame.Ambient_Light, 1.0, 1.0, 1.0);

   /* Init atmospheric stuff. [DB 12/94] */

   Frame.Atmosphere = NULL;

   Frame.Fog = NULL;

   Frame.Rainbow = NULL;

   Frame.Skysphere = NULL;
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

void Parse_Begin ()
  {
   char *front;

   Brace_Stack[++Brace_Index]=Token.Token_Id;

   Get_Token ();

   if (Token.Token_Id == LEFT_CURLY_TOKEN)
     return;

   front = Get_Token_String (Brace_Stack[Brace_Index]);

   Where_Error ();
   Error_Line ("Missing { after %s, ", front);
   Found_Instead ();
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

void Parse_End ()
  {
   char *front;

   Get_Token ();

   if (Token.Token_Id == RIGHT_CURLY_TOKEN)
     {
      Brace_Index--;
      return;
     }

   front = Get_Token_String (Brace_Stack[Brace_Index]);

   Where_Error ();
   Error_Line("No matching } in %s,", front);
   Found_Instead ();
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

static OBJECT *Parse_Object_Id ()
  {
   OBJECT *Object;

   EXPECT
     CASE (OBJECT_ID_TOKEN)
       Warn_State(OBJECT_ID_TOKEN, OBJECT_TOKEN);
       Object = Copy_Object((OBJECT *) Token.Constant_Data);
       Parse_Object_Mods (Object);
       EXIT
     END_CASE

     OTHERWISE
       Object = NULL;
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   return (Object);
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

void Parse_Comma ()
  {
   Get_Token();
   if (Token.Token_Id != COMMA_TOKEN)
     {
      UNGET;
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

static void Parse_Coeffs(order, Coeffs)
  int order;
  DBL *Coeffs;
  {
   int i;

   EXPECT
     CASE (LEFT_ANGLE_TOKEN)
       Coeffs[0] = Parse_Float();
       for (i = 1; i < term_counts(order); i++)
         {
          Parse_Comma();
          Coeffs[i] = Parse_Float();
         }
       GET (RIGHT_ANGLE_TOKEN);
       EXIT
     END_CASE

     OTHERWISE
       Parse_Error (LEFT_ANGLE_TOKEN);
     END_CASE
   END_EXPECT
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

static
OBJECT *Parse_Bound_Clip ()
  {
   VECTOR Local_Vector;
   MATRIX Local_Matrix;
   TRANSFORM Local_Trans;
   OBJECT *First, *Current, *Prev;

   First = Prev = NULL;

   while ((Current = Parse_Object ()) != NULL)
     {
      if (Current->Type & (TEXTURED_OBJECT+PATCH_OBJECT))
        Error ("Illegal texture or patch in clip or bound.");
      if (First == NULL)
        First = Current;
      if (Prev != NULL)
        Prev->Sibling = Current;
      Prev = Current;
     }

   EXPECT
     CASE (TRANSLATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Translation_Transform(&Local_Trans, Local_Vector);
       for (Current = First; Current != NULL; Current = Current->Sibling)
       {
         Translate_Object (Current, Local_Vector, &Local_Trans);
       }
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Rotation_Transform(&Local_Trans, Local_Vector);
       for (Current = First; Current != NULL; Current = Current->Sibling)
       {
         Rotate_Object (Current, Local_Vector, &Local_Trans);
       }
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (Local_Vector);
       Compute_Scaling_Transform(&Local_Trans, Local_Vector);
       for (Current = First; Current != NULL; Current = Current->Sibling)
       {
         Scale_Object (Current, Local_Vector, &Local_Trans);
       }
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       for (Current = First; Current != NULL; Current = Current->Sibling)
       {
         Transform_Object (Current, (TRANSFORM *)Token.Constant_Data);
       }
     END_CASE

     CASE (MATRIX_TOKEN)
       Parse_Matrix (Local_Matrix);
       Compute_Matrix_Transform(&Local_Trans, Local_Matrix);
       for (Current = First; Current != NULL; Current = Current->Sibling)
       {
         Transform_Object (Current, &Local_Trans);
       }
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   if (First==NULL)
   {
      Parse_Error_Str("object");
   }

   return (First);
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

void Parse_Object_Mods (Object)
  OBJECT *Object;
  {
   DBL Temp_Water_Level;
   DBL V1, V2;
   VECTOR Min, Max;  
   VECTOR Local_Vector;
   MATRIX Local_Matrix;
   TRANSFORM Local_Trans;
   BBOX BBox;
   OBJECT *Sib;
   TEXTURE *Local_Texture;
   OBJECT *Temp1_Object;
   OBJECT *Temp2_Object;
   COLOUR Local_Colour;

   EXPECT
     CASE_COLOUR
       Parse_Colour (Local_Colour);
       if (opts.Language_Version < 1.5)
         if (Object->Texture != NULL)
           if (Object->Texture->Type == PLAIN_PATTERN)
             if (opts.Quality_Flags & Q_QUICKC)
             {
              Assign_Colour(Object->Texture->Pigment->Colour,Local_Colour);
              break;  /* acts like END_CASE */
             }
       Warn(0.0, "Quick color belongs in texture. Color ignored.");
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Translation_Transform(&Local_Trans, Local_Vector);
       Translate_Object (Object, Local_Vector, &Local_Trans);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Rotation_Transform(&Local_Trans, Local_Vector);
       Rotate_Object (Object, Local_Vector, &Local_Trans);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (Local_Vector);
       Compute_Scaling_Transform(&Local_Trans, Local_Vector);
       Scale_Object (Object, Local_Vector, &Local_Trans);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Object (Object, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     CASE (MATRIX_TOKEN)
       Parse_Matrix (Local_Matrix);
       Compute_Matrix_Transform(&Local_Trans, Local_Matrix);
       Transform_Object (Object, &Local_Trans);
     END_CASE

     CASE (BOUNDED_BY_TOKEN)
       Parse_Begin ();
       if (Object->Bound != NULL)
         if (Object->Clip == Object->Bound)
           Error ("Cannot add bounds after linking bounds and clips.");

       EXPECT
         CASE (CLIPPED_BY_TOKEN)
           if (Object->Bound != NULL)
             Error ("Cannot link clips with previous bounds.");
           Object->Bound = Object->Clip;
           EXIT
         END_CASE

         OTHERWISE
           UNGET
           Temp1_Object = Temp2_Object = Parse_Bound_Clip ();
           while (Temp2_Object->Sibling != NULL)
             Temp2_Object = Temp2_Object->Sibling;
           Temp2_Object->Sibling = Object->Bound;
           Object->Bound = Temp1_Object;
           EXIT
         END_CASE
       END_EXPECT

       Parse_End ();
     END_CASE

     CASE (CLIPPED_BY_TOKEN)
       Parse_Begin ();
       if (Object->Clip != NULL)
         if (Object->Clip == Object->Bound)
           Error ("Cannot add clips after linking bounds and clips.");

       EXPECT
         CASE (BOUNDED_BY_TOKEN)
           if (Object->Clip != NULL)
             Error ("Cannot link bounds with previous clips.");
           Object->Clip = Object->Bound;
           EXIT
         END_CASE

         OTHERWISE
           UNGET
           Temp1_Object = Temp2_Object = Parse_Bound_Clip ();
           while (Temp2_Object->Sibling != NULL)
             Temp2_Object = Temp2_Object->Sibling;
           Temp2_Object->Sibling = Object->Clip;
           Object->Clip = Temp1_Object;

           /* Compute quadric bounding box before transformations. [DB 8/94] */

           if (Object->Methods == &Quadric_Methods)
           {
             Make_Vector(Min, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);
             Make_Vector(Max,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);

             Compute_Quadric_BBox((QUADRIC *)Object, Min, Max);
           }
           EXIT
         END_CASE
       END_EXPECT

       Parse_End ();
     END_CASE

     CASE (TEXTURE_TOKEN)
       Object->Type |= TEXTURED_OBJECT;
       Parse_Begin ();
       Local_Texture = Parse_Texture ();
       Parse_End ();
       Link_Textures(&(Object->Texture), Local_Texture);
     END_CASE

     CASE4 (PIGMENT_TOKEN, TNORMAL_TOKEN, FINISH_TOKEN, HALO_TOKEN)
       Object->Type |= TEXTURED_OBJECT;
       if (Object->Texture == NULL)
         Object->Texture = Copy_Textures(Default_Texture);
       else
         if (Object->Texture->Type != PLAIN_PATTERN)
           Link_Textures(&(Object->Texture), Copy_Textures(Default_Texture));
       UNGET
       EXPECT
         CASE (PIGMENT_TOKEN)
           Parse_Begin ();
           Parse_Pigment ( &(Object->Texture->Pigment) );
           Parse_End ();
         END_CASE

         CASE (TNORMAL_TOKEN)
           Parse_Begin ();
           Parse_Tnormal ( &(Object->Texture->Tnormal) );
           Parse_End ();
         END_CASE

         CASE (FINISH_TOKEN)
           Parse_Finish ( &(Object->Texture->Finish) );
         END_CASE

         CASE (HALO_TOKEN)
           Parse_Halo ( &(Object->Texture->Halo) );
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT
     END_CASE

     CASE (INVERSE_TOKEN)
       if (Object->Type & PATCH_OBJECT)
         Warn (0.0, "Cannot invert a patch object.");
       Invert_Object (Object);
     END_CASE

     CASE (STURM_TOKEN)
       if (!(Object->Type & STURM_OK_OBJECT))
         Not_With ("sturm","this object");
       Bool_Flag (Object, STURM_FLAG, (Allow_Float(1.0) > 0.0));
     END_CASE

     CASE (WATER_LEVEL_TOKEN)
       if (!(Object->Type & WATER_LEVEL_OK_OBJECT))
         Not_With ("water_level","this object");
       Temp_Water_Level = Parse_Float();
       if (opts.Language_Version < 2.0)
         Temp_Water_Level /=256.0;
       ((HFIELD *) Object)->bounding_box->bounds[0][Y] = 65536.0 * Temp_Water_Level;
     END_CASE

     CASE (SMOOTH_TOKEN)
       if (!(Object->Type & SMOOTH_OK_OBJECT))
         Not_With ("smooth","this object");
       Set_Flag(Object, SMOOTHED_FLAG);
       Object->Type |= DOUBLE_ILLUMINATE;
     END_CASE

     CASE (NO_SHADOW_TOKEN)
       Set_Flag(Object, NO_SHADOW_FLAG);
     END_CASE

     CASE (LIGHT_SOURCE_TOKEN)
       Error("Light source must be defined using new syntax.");
     END_CASE

     CASE(HIERARCHY_TOKEN)
       if (!(Object->Type & HIERARCHY_OK_OBJECT))
         Not_With ("hierarchy", "this object");
       Bool_Flag (Object, HIERARCHY_FLAG, (Allow_Float(1.0) > 0.0));
     END_CASE

     CASE(HOLLOW_TOKEN)
       Bool_Flag (Object, HOLLOW_FLAG, (Allow_Float(1.0) > 0.0));
       Set_Flag (Object, HOLLOW_SET_FLAG);
       if ((Object->Methods == &CSG_Intersection_Methods) ||
           (Object->Methods == &CSG_Merge_Methods) ||
           (Object->Methods == &CSG_Union_Methods))
       {
         Set_CSG_Children_Hollow(Object, Test_Flag(Object, HOLLOW_FLAG));
       }
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   /*
    * Assign bounding objects' bounding box to object
    * if object's bounding box is larger. [DB 9/94]
    */

   if (Object->Bound != NULL)
   {
     /* Get bounding objects bounding box. */

     Make_Vector(Min, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);
     Make_Vector(Max,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);

     for (Sib = Object->Bound; Sib != NULL; Sib = Sib->Sibling)
     {
       if (!Test_Flag(Sib, INVERTED_FLAG))
       {
         Min[X] = max(Min[X], Sib->BBox.Lower_Left[X]);
         Min[Y] = max(Min[Y], Sib->BBox.Lower_Left[Y]);
         Min[Z] = max(Min[Z], Sib->BBox.Lower_Left[Z]);
         Max[X] = min(Max[X], Sib->BBox.Lower_Left[X] + Sib->BBox.Lengths[X]);
         Max[Y] = min(Max[Y], Sib->BBox.Lower_Left[Y] + Sib->BBox.Lengths[Y]);
         Max[Z] = min(Max[Z], Sib->BBox.Lower_Left[Z] + Sib->BBox.Lengths[Z]);
       }
     }

     Make_BBox_from_min_max(BBox, Min, Max);

     /* Get bounding boxes' volumes. */

     BOUNDS_VOLUME(V1, BBox);
     BOUNDS_VOLUME(V2, Object->BBox);

     if (V1 < V2)
     {
       Object->BBox = BBox;
     }
   }

   /*
    * Assign clipping objects' bounding box to object
    * if object's bounding box is larger. [DB 9/94]
    */

   if (Object->Clip != NULL)
   {
     /* Get clipping objects bounding box. */

     Make_Vector(Min, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);
     Make_Vector(Max,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);

     for (Sib = Object->Clip; Sib != NULL; Sib = Sib->Sibling)
     {
       if (!Test_Flag(Sib, INVERTED_FLAG))
       {
         Min[X] = max(Min[X], Sib->BBox.Lower_Left[X]);
         Min[Y] = max(Min[Y], Sib->BBox.Lower_Left[Y]);
         Min[Z] = max(Min[Z], Sib->BBox.Lower_Left[Z]);
         Max[X] = min(Max[X], Sib->BBox.Lower_Left[X] + Sib->BBox.Lengths[X]);
         Max[Y] = min(Max[Y], Sib->BBox.Lower_Left[Y] + Sib->BBox.Lengths[Y]);
         Max[Z] = min(Max[Z], Sib->BBox.Lower_Left[Z] + Sib->BBox.Lengths[Z]);
       }
     }

     Make_BBox_from_min_max(BBox, Min, Max);

     /* Get bounding boxes' volumes. */

     BOUNDS_VOLUME(V1, BBox);
     BOUNDS_VOLUME(V2, Object->BBox);

     if (V1 < V2)
     {
       Object->BBox = BBox;
     }
   }

   Parse_End ();
  }



/*****************************************************************************
*
* FUNCTION
*
*  Set_CSG_Children_Hollow
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

static void Set_CSG_Children_Hollow(Object, hollow)
OBJECT *Object;
int hollow;
{
  OBJECT *Sib;

  for (Sib = ((CSG *)Object)->Children; Sib != NULL; Sib = Sib->Sibling)
  {
    if (!Test_Flag(Sib, HOLLOW_SET_FLAG))
    {
      if ((Sib->Methods == &CSG_Intersection_Methods) ||
          (Sib->Methods == &CSG_Merge_Methods) ||
          (Sib->Methods == &CSG_Union_Methods))
      {
        Set_CSG_Children_Hollow(Sib, hollow);
      }
      else
      {
        Sib->Flags = (Sib->Flags & (~HOLLOW_FLAG)) | hollow;
      }
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

static OBJECT *Parse_Sphere()
{
  SPHERE *Object;

  Parse_Begin();

  if ((Object = (SPHERE *)Parse_Object_Id()) != NULL)
  {
    return ((OBJECT *) Object);
  }

  Object = Create_Sphere();

  Parse_Vector(Object->Center);

  Parse_Comma();

  Object->Radius = Parse_Float();

  Compute_Sphere_BBox(Object);  

  Parse_Object_Mods((OBJECT *)Object);

  return((OBJECT *)Object);
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

static
OBJECT *Parse_Plane ()
  {
   DBL len;
   PLANE *Object;

   Parse_Begin ();

   if ( (Object = (PLANE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Plane();

   Parse_Vector(Object->Normal_Vector);   Parse_Comma();
   VLength(len, Object->Normal_Vector);
   if (len < EPSILON)
   {
     Error("Degenerate plane normal.");
   }
   VInverseScaleEq(Object->Normal_Vector, len);
   Object->Distance = -Parse_Float();

   Compute_Plane_BBox(Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_HField ()
{
  HFIELD *Object;
  VECTOR Local_Vector;
  IMAGE *Image;

  Parse_Begin ();

  if ( (Object = (HFIELD *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_HField();

   Image = Parse_Image (HF_FILE);
   Image->Use_Colour_Flag = FALSE;

   Make_Vector(Object->bounding_box->bounds[0], 0.0, 0.0, 0.0);

   if (Image->File_Type == POT_FILE)
   {
     Object->bounding_box->bounds[1][X] = Image->width/2.0 - 1.0;
   }
   else
   {
     Object->bounding_box->bounds[1][X] = Image->width - 1.0;
   }

   Object->bounding_box->bounds[1][Y] = 65536.0;
   Object->bounding_box->bounds[1][Z] = Image->height - 1.0;

   Make_Vector(Local_Vector,
     1.0 / (Object->bounding_box->bounds[1][X]),
     1.0 / (Object->bounding_box->bounds[1][Y]),
     1.0 / (Object->bounding_box->bounds[1][Z]));

   Compute_Scaling_Transform(Object->Trans, Local_Vector);

   Parse_Object_Mods ((OBJECT *)Object);

   Compute_HField(Object, Image);

   Compute_HField_BBox(Object);

   Destroy_Image (Image);

   return ((OBJECT *) Object);
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

static OBJECT *Parse_Triangle()
{
  TRIANGLE *Object;

  Parse_Begin();

  if ((Object = (TRIANGLE *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *) Object);
  }

  Object = Create_Triangle();

  Parse_Vector(Object->P1);    Parse_Comma();
  Parse_Vector(Object->P2);    Parse_Comma();
  Parse_Vector(Object->P3);

  /* Note that Compute_Triangle also computes the bounding box. */

  if (!Compute_Triangle(Object, FALSE))
  {
    Warn(0.0, "Degenerate triangle. Please remove.");
  }

  Parse_Object_Mods((OBJECT *)Object);

  return((OBJECT *)Object);
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

static
OBJECT *Parse_Smooth_Triangle ()
  {
   SMOOTH_TRIANGLE *Object;
   short degen;
   DBL vlen;

   degen=FALSE;

   Parse_Begin ();

   if ( (Object = (SMOOTH_TRIANGLE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Smooth_Triangle();

   Parse_Vector (Object->P1);    Parse_Comma();
   Parse_Vector (Object->N1);    Parse_Comma();

   VLength(vlen,Object->N1);

   if (vlen == 0.0)
     degen=TRUE;
   else
     VNormalize (Object->N1, Object->N1);

   Parse_Vector (Object->P2);    Parse_Comma();
   Parse_Vector (Object->N2);    Parse_Comma();

   VLength(vlen,Object->N2);

   if (vlen == 0.0)
     degen=TRUE;
   else
     VNormalize (Object->N2, Object->N2);

   Parse_Vector (Object->P3);    Parse_Comma();
   Parse_Vector (Object->N3);

   VLength(vlen,Object->N3);

   if (vlen == 0.0)
     degen=TRUE;
   else
     VNormalize (Object->N3, Object->N3);

   if (!degen)
   {
     degen=!Compute_Triangle ((TRIANGLE *) Object,TRUE);
   }

   if (degen)
   {
     Warn(0.0, "Degenerate triangle. Please remove.");
   }

   Compute_Triangle_BBox((TRIANGLE *)Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_Quadric ()
  {
   VECTOR Min, Max;
   QUADRIC *Object;

   Parse_Begin ();

   if ( (Object = (QUADRIC *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Quadric();

        Parse_Vector(Object->Square_Terms);     Parse_Comma();
        Parse_Vector(Object->Mixed_Terms);      Parse_Comma();
        Parse_Vector(Object->Terms);            Parse_Comma();
   Object->Constant = Parse_Float();

   Make_Vector(Min, -BOUND_HUGE, -BOUND_HUGE, -BOUND_HUGE);
   Make_Vector(Max,  BOUND_HUGE,  BOUND_HUGE,  BOUND_HUGE);

   Compute_Quadric_BBox(Object, Min, Max);  

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_Box ()
  {
   BOX *Object;
   DBL temp;

   Parse_Begin ();

   if ( (Object = (BOX *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Box();

        Parse_Vector(Object->bounds[0]);     Parse_Comma();
        Parse_Vector(Object->bounds[1]);

    if (Object->bounds[0][X] > Object->bounds[1][X]) {
       temp = Object->bounds[0][X];
       Object->bounds[0][X] = Object->bounds[1][X];
       Object->bounds[1][X] = temp;
       }
    if (Object->bounds[0][Y] > Object->bounds[1][Y]) {
       temp = Object->bounds[0][Y];
       Object->bounds[0][Y] = Object->bounds[1][Y];
       Object->bounds[1][Y] = temp;
       }
    if (Object->bounds[0][Z] > Object->bounds[1][Z]) {
       temp = Object->bounds[0][Z];
       Object->bounds[0][Z] = Object->bounds[1][Z];
       Object->bounds[1][Z] = temp;
       }

   Compute_Box_BBox(Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_Disc ()
  {
   DISC *Object;
   DBL tmpf;

   Parse_Begin ();

   if ( (Object = (DISC *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Disc();

        Parse_Vector(Object->center); Parse_Comma ();
        Parse_Vector(Object->normal); Parse_Comma ();
   VNormalize(Object->normal, Object->normal);

   tmpf = Parse_Float(); Parse_Comma ();
   Object->oradius2 = tmpf * tmpf;

   EXPECT
     CASE_FLOAT
       tmpf = Parse_Float();
       Object->iradius2 = tmpf * tmpf;
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   /* Calculate info needed for ray-disc intersections */
   VDot(tmpf, Object->center, Object->normal);
   Object->d = -tmpf;

   Compute_Disc(Object);  

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_Cylinder ()
  {
   CONE *Object;

   Parse_Begin ();

   if ( (Object = (CONE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Cylinder();

        Parse_Vector(Object->apex);  Parse_Comma ();
        Parse_Vector(Object->base);  Parse_Comma ();
   Object->apex_radius = Parse_Float();
   Object->base_radius = Object->apex_radius;

   EXPECT
     CASE(OPEN_TOKEN)
       Clear_Flag(Object, CLOSED_FLAG);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Compute_Cylinder_Data((OBJECT *)Object);

   Compute_Cone_BBox(Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_Cone ()
  {
   CONE *Object;

   Parse_Begin ();

   if ( (Object = (CONE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Cone();

        Parse_Vector(Object->apex);  Parse_Comma ();
        Object->apex_radius = Parse_Float();  Parse_Comma ();

        Parse_Vector(Object->base);  Parse_Comma ();
   Object->base_radius = Parse_Float();

   EXPECT
     CASE(OPEN_TOKEN)
       Clear_Flag(Object, CLOSED_FLAG);
       EXIT
     END_CASE
     
     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   /* Compute run-time values for the cone */
   Compute_Cone_Data((OBJECT *)Object);

   Compute_Cone_BBox(Object);  

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
  }



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Blob_Element_Mods
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
*   -
*
* CHANGES
*
*   Sep 1994 : Creation.
*
******************************************************************************/

static void Parse_Blob_Element_Mods(Element)
BLOB_ELEMENT *Element;
{
  VECTOR Local_Vector;
  MATRIX Local_Matrix;
  TRANSFORM Local_Trans;
  TEXTURE *Local_Texture;

  EXPECT
    CASE (TRANSLATE_TOKEN)
      Parse_Vector (Local_Vector);
      Translate_Blob_Element (Element, Local_Vector);
    END_CASE

    CASE (ROTATE_TOKEN)
      Parse_Vector (Local_Vector);
      Rotate_Blob_Element (Element, Local_Vector);
    END_CASE

    CASE (SCALE_TOKEN)
      Parse_Scale_Vector (Local_Vector);
      Scale_Blob_Element (Element, Local_Vector);
    END_CASE

    CASE (TRANSFORM_TOKEN)
      GET(TRANSFORM_ID_TOKEN)
      Transform_Blob_Element (Element, (TRANSFORM *)Token.Constant_Data);
    END_CASE

    CASE (MATRIX_TOKEN)
      Parse_Matrix (Local_Matrix);
      Compute_Matrix_Transform(&Local_Trans, Local_Matrix);
      Transform_Blob_Element (Element, &Local_Trans);
    END_CASE

    CASE (TEXTURE_TOKEN)
      Parse_Begin ();
      Local_Texture = Parse_Texture();
      Parse_End ();
      Link_Textures(&Element->Texture, Local_Texture);
    END_CASE

    CASE4 (PIGMENT_TOKEN, TNORMAL_TOKEN, FINISH_TOKEN, HALO_TOKEN)
      if (Element->Texture == NULL)
      {
        Element->Texture = Copy_Textures(Default_Texture);
      }
      else
      {
        if (Element->Texture->Type != PLAIN_PATTERN)
        {
          Link_Textures(&Element->Texture, Copy_Textures(Default_Texture));
        }
      }
      UNGET
      EXPECT
        CASE (PIGMENT_TOKEN)
          Parse_Begin ();
          Parse_Pigment(&Element->Texture->Pigment);
          Parse_End ();
        END_CASE

        CASE (TNORMAL_TOKEN)
          Parse_Begin ();
          Parse_Tnormal(&Element->Texture->Tnormal);
          Parse_End ();
        END_CASE

        CASE (FINISH_TOKEN)
          Parse_Finish(&Element->Texture->Finish);
        END_CASE

        CASE (HALO_TOKEN)
          Parse_Halo(&(Element->Texture->Halo));
        END_CASE

        OTHERWISE
          UNGET
          EXIT
        END_CASE
      END_EXPECT
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  Parse_End();

  /* Postprocess to make sure that HAS_FILTER will be set correctly. */

  Post_Textures(Element->Texture);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Blob
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
*   -
*
* CHANGES
*
*   Jul 1994 : Creation.
*
******************************************************************************/

static OBJECT *Parse_Blob()
{
  int npoints;
  DBL threshold;
  VECTOR Axis, Base, Apex;
  BLOB *Object;
  BLOB_LIST *blob_components, *blob_component;

  Parse_Begin();

  if ((Object = (BLOB *)Parse_Object_Id()) != NULL)
  {
    return ((OBJECT *) Object);
  }

  Object = Create_Blob();

  blob_components = NULL;

  npoints = 0;

  threshold = 1.0;

  EXPECT
    CASE (THRESHOLD_TOKEN)
      threshold = Parse_Float();
    END_CASE

    /*************************************************************************
     * Read sperical component (old syntax).
     *************************************************************************/

    CASE (COMPONENT_TOKEN)
      blob_component = Create_Blob_List_Element();

      blob_component->elem.Type = BLOB_SPHERE;

      blob_component->elem.c[2] = Parse_Float();

      Parse_Comma();

      blob_component->elem.rad2 = Parse_Float();

      Parse_Comma();

      blob_component->elem.rad2 = Sqr(blob_component->elem.rad2);

      Parse_Vector(blob_component->elem.O);

      /* Next component. */

      blob_component->next = blob_components;

      blob_components = blob_component;

      npoints++;
    END_CASE

    /*************************************************************************
     * Read sperical component (new syntax).
     *************************************************************************/

    CASE (SPHERE_TOKEN)
      blob_component = Create_Blob_List_Element();

      blob_component->elem.Type = BLOB_SPHERE;

      Parse_Begin();

      Parse_Vector(blob_component->elem.O);

      Parse_Comma();

      blob_component->elem.rad2 = Parse_Float();

      blob_component->elem.rad2 = Sqr(blob_component->elem.rad2);

      Parse_Comma();

      ALLOW(STRENGTH_TOKEN)

      blob_component->elem.c[2] = Parse_Float();

      Parse_Blob_Element_Mods(&blob_component->elem);

      /* Next component. */

      blob_component->next = blob_components;

      blob_components = blob_component;

      npoints++;
    END_CASE

    /*************************************************************************
     * Read cylindrical component.
     *************************************************************************/

    CASE (CYLINDER_TOKEN)
      blob_component = Create_Blob_List_Element();

      blob_component->elem.Type = BLOB_CYLINDER;

      blob_component->elem.Trans = Create_Transform();

      Parse_Begin();

      Parse_Vector(Base);

      Parse_Comma();

      Parse_Vector(Apex);

      Parse_Comma();

      blob_component->elem.rad2 = Parse_Float();

      blob_component->elem.rad2 = Sqr(blob_component->elem.rad2);

      Parse_Comma();

      ALLOW(STRENGTH_TOKEN)

      blob_component->elem.c[2] = Parse_Float();

      /* Calculate cylinder's coordinate system. */

      VSub(Axis, Apex, Base);

      VLength(blob_component->elem.len, Axis);

      if (blob_component->elem.len < EPSILON)
      {
        Error("Degenerate cylindrical component in blob.\n");
      }

      VInverseScaleEq(Axis, blob_component->elem.len);

      Compute_Coordinate_Transform(blob_component->elem.Trans, Base, Axis, 1.0, 1.0);

      Parse_Blob_Element_Mods(&blob_component->elem);

      /* Next component. */

      blob_component->next = blob_components;

      blob_components = blob_component;

      npoints++;
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  Create_Blob_Element_Texture_List(Object, blob_components, npoints);

  Parse_Object_Mods((OBJECT *)Object);

  /* The blob's texture has to be processed before Make_Blob() is called. */

  Post_Textures(Object->Texture);
  
  /* Finally, process the information */

  Make_Blob(Object, threshold, blob_components, npoints);

  return((OBJECT *)Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Julia_Fractal
*
* INPUT None
*   
* OUTPUT Fractal Objecstructure filledt
*   
* RETURNS 
*
*   OBJECT * -
*   
* AUTHOR
*
*   Pascal Massimino
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Dec 1994 : Adopted to version 3.0. [DB]
*   Sept 1995 : Total rewrite for new syntax [TW]
*
******************************************************************************/

static OBJECT *Parse_Julia_Fractal ()
{
  FRACTAL *Object;
  DBL P;

  Parse_Begin();

  if ( (Object = (FRACTAL *)Parse_Object_Id()) != NULL)
    return((OBJECT *)Object);

  Object = Create_Fractal();

  Parse_Vector4D(Object->Julia_Parm); 

  EXPECT

    CASE(MAX_ITERATION_TOKEN)
      Object->n = (int)floor(Parse_Float()); 

      if (Object->n <= 0)
      {
        Object->n = 1;
      }
    END_CASE

    CASE(SLICE_TOKEN)
      Parse_Vector4D(Object->Slice);
      Parse_Comma();
      Object->SliceDist = Parse_Float(); 

      /* normalize slice vector */
      V4D_Dot(P,Object->Slice, Object->Slice);
      if (fabs(P) < EPSILON)
      {
        Error("Slice vector is zero.\n");
      }
      if (fabs(Object->Slice[T]) < EPSILON)
      {
        Error("Slice t component is zero.\n");
      }
      P = sqrt(P);
      V4D_InverseScaleEq(Object->Slice, P);      

    END_CASE

    CASE(PRECISION_TOKEN)
      P = Parse_Float(); 
      if ( P < 1.0 )
      {
        P = 1.0;
      }
      Object->Precision = 1.0 / P;
    END_CASE
      
    CASE(FLOAT_FUNCT_TOKEN)
      switch(Token.Function_Id)
      {
        case EXP_TOKEN:
          Object->Sub_Type = EXP_STYPE;
          break;
        case LOG_TOKEN:
          Object->Sub_Type = LOG_STYPE;
          break;
        case SIN_TOKEN:
          Object->Sub_Type = SIN_STYPE;
          break;
        case ASIN_TOKEN:
          Object->Sub_Type = ASIN_STYPE;
          break;
        case COS_TOKEN:
          Object->Sub_Type = COS_STYPE;
          break;
        case ACOS_TOKEN:
          Object->Sub_Type = ACOS_STYPE;
          break;
        default: Parse_Error_Str ("fractal keyword");
      }    
    END_CASE

    /* if any of the next become supported by the expression parser,
     * then their handling would need to move above to the FUNC_TOKEN
     * case above.
     */
    CASE(ATAN_TOKEN)
      Object->Sub_Type = ATAN_STYPE;
    END_CASE

    CASE(COSH_TOKEN)
      Object->Sub_Type = COSH_STYPE;
    END_CASE

    CASE(SINH_TOKEN)
      Object->Sub_Type = SINH_STYPE;
    END_CASE

    CASE(TANH_TOKEN)
      Object->Sub_Type = TANH_STYPE;
    END_CASE

    CASE(ATANH_TOKEN)
      Object->Sub_Type = ATANH_STYPE;
    END_CASE

    CASE(ACOSH_TOKEN)
      Object->Sub_Type = ACOSH_STYPE;
    END_CASE

    CASE(ASINH_TOKEN)
      Object->Sub_Type = ASINH_STYPE;
    END_CASE

    CASE(SQR_TOKEN)
      Object->Sub_Type = SQR_STYPE;
    END_CASE

    CASE(PWR_TOKEN)
      Object->Sub_Type = PWR_STYPE;
      Parse_Float_Param2(&Object->exponent.x,&Object->exponent.y);
    END_CASE

    CASE(CUBE_TOKEN)
      Object->Sub_Type = CUBE_STYPE;
    END_CASE

    CASE(RECIPROCAL_TOKEN)
      Object->Sub_Type = RECIPROCAL_STYPE;
    END_CASE

    CASE(HYPERCOMPLEX_TOKEN)
      Object->Algebra = HYPERCOMPLEX_TYPE;
    END_CASE

    CASE(QUATERNION_TOKEN)
      Object->Algebra = QUATERNION_TYPE;
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE

  END_EXPECT

  Parse_Object_Mods((OBJECT *)Object);

  SetUp_Fractal(Object);

  return((OBJECT *)Object);
}




/*****************************************************************************
*
* FUNCTION
*
*   Parse_Polygon
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   OBJECT * -
*
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   May 1994 : Creation.
*
*   Oct 1994 : Modified to use new polygon data structure. [DB]
*
******************************************************************************/

static OBJECT *Parse_Polygon()
{
  int i, closed = FALSE;
  int Number;
  POLYGON *Object;
  VECTOR *Points, P;

  Parse_Begin();

  if ((Object = (POLYGON *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *) Object);
  }

  Object = Create_Polygon();

  Number = (int)Parse_Float();

  if (Number < 3)
  {
    Error("Polygon needs at least three points.");
  }

  Points = (VECTOR *)POV_MALLOC((Number+1)*sizeof(VECTOR), "temporary polygon points");

  for (i = 0; i < Number; i++)
  {
    Parse_Comma();

    Parse_Vector(Points[i]);
  }

  /* Check for closed polygons. */

  Assign_Vector(P, Points[0]);

  for (i = 1; i < Number; i++)
  {
    closed = FALSE;

    if ((fabs(P[X] - Points[i][X]) < EPSILON) &&
        (fabs(P[Y] - Points[i][Y]) < EPSILON) &&
        (fabs(P[Z] - Points[i][Z]) < EPSILON))
    {
      i++;

      if (i < Number)
      {
        Assign_Vector(P, Points[i]);
      }

      closed = TRUE;
    }
  }

  if (!closed)
  {
    Warn(0.0, "Polygon not closed. Closing it.");

    Assign_Vector(Points[Number], P);

    Number++;
  }

  Compute_Polygon(Object, Number, Points);

  POV_FREE (Points);

  Parse_Object_Mods ((OBJECT *)Object);

  return((OBJECT *) Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Prism
*
* INPUT
*
* OUTPUT
*
* RETURNS
*
*   OBJECT * -
*
* AUTHOR
*
*   Dieter Bayer
*
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

static OBJECT *Parse_Prism()
{
  int i, closed = FALSE;
  DBL h;

  PRISM *Object;
  UV_VECT *Points, P;

  Parse_Begin();

  if ((Object = (PRISM *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *) Object);
  }

  Object = Create_Prism();

  /* 
   * Determine kind of spline used (linear, quadratic, cubic) 
   * and type of sweeping (linear, conic).
   */

  EXPECT
    CASE(LINEAR_SPLINE_TOKEN)
      Object->Spline_Type = LINEAR_SPLINE;
    END_CASE

    CASE(QUADRATIC_SPLINE_TOKEN)
      Object->Spline_Type = QUADRATIC_SPLINE;
    END_CASE

    CASE(CUBIC_SPLINE_TOKEN)
      Object->Spline_Type = CUBIC_SPLINE;
    END_CASE

    CASE(LINEAR_SWEEP_TOKEN)
      Object->Sweep_Type = LINEAR_SWEEP;
    END_CASE

    CASE(CONIC_SWEEP_TOKEN)
      Object->Sweep_Type = CONIC_SWEEP;
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  /* Read prism heights. */

  Object->Height1 = Parse_Float(); Parse_Comma();
  Object->Height2 = Parse_Float(); Parse_Comma();

  if (Object->Height1 > Object->Height2)
  {
    h = Object->Height1;
    Object->Height1 = Object->Height2;
    Object->Height2 = h;
  }

  /* Get number of points = number of segments. */

  Object->Number = (int)Parse_Float();

  switch (Object->Spline_Type)
  {
    case LINEAR_SPLINE :

      if (Object->Number < 3)
      {
        Error("Prism with linear splines must have at least three points.");
      }

      break;

    case QUADRATIC_SPLINE :

      if (Object->Number < 5)
      {
        Error("Prism with quadratic splines must have at least five points.");
      }

      break;

    case CUBIC_SPLINE :

      if (Object->Number < 6)
      {
        Error("Prism with cubic splines must have at least six points.");
      }
  }

  /* Allocate Object->Number points for the prism. */

  Points = (UV_VECT *)POV_MALLOC((Object->Number+1) * sizeof(UV_VECT), "temporary prism points");

  /* Read points (x, y : coordinate of 2d point; z : not used). */

  for (i = 0; i < Object->Number; i++)
  {
    Parse_Comma();

    Parse_UV_Vect(Points[i]);
  }

  /* Closed or not closed that's the question. */

  EXPECT
    CASE(OPEN_TOKEN)
      Clear_Flag(Object, CLOSED_FLAG);
      EXIT
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  /* Check for closed prism. */

  switch (Object->Spline_Type)
  {
    case LINEAR_SPLINE :

      i = 1;

      Assign_UV_Vect(P, Points[0]);

      break;

    case QUADRATIC_SPLINE :
    case CUBIC_SPLINE :

      i = 2;

      Assign_UV_Vect(P, Points[1]);

      break;
  }

  for ( ; i < Object->Number; i++)
  {
    closed = FALSE;

    if ((fabs(P[X] - Points[i][X]) < EPSILON) &&
        (fabs(P[Y] - Points[i][Y]) < EPSILON))
    {
      switch (Object->Spline_Type)
      {
        case LINEAR_SPLINE :

          i++;

          if (i < Object->Number)
          {
            Assign_UV_Vect(P, Points[i]);
          }

          break;

        case QUADRATIC_SPLINE :

          i += 2;

          if (i < Object->Number)
          {
            Assign_UV_Vect(P, Points[i]);
          }

          break;

        case CUBIC_SPLINE :

          i += 3;

          if (i < Object->Number)
          {
            Assign_UV_Vect(P, Points[i]);
          }

          break;
      }

      closed = TRUE;
    }
  }

  if (!closed)
  {
    if (Object->Spline_Type == LINEAR_SPLINE)
    {
      Assign_UV_Vect(Points[Object->Number], P);

      Object->Number++;

      Warn(0.0, "Linear prism not closed. Closing it.");
    }
    else
    {
      Set_Flag(Object, DEGENERATE_FLAG);

      Warn(0.0, "Prism not closed. Ignoring it.");
    }
  }

  /* Compute spline segments. */

  Compute_Prism(Object, Points);

  /* Compute bounding box. */

  Compute_Prism_BBox(Object);

  /* Parse object's modifiers. */

  Parse_Object_Mods((OBJECT *)Object);

  /* Destroy temporary points. */

  POV_FREE (Points);

  return((OBJECT *) Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Sor
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   OBJECT * -
*
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Read a surface of revolution primitive.
*
* CHANGES
*
*   May 1994 : Creation.
*
******************************************************************************/

static OBJECT *Parse_Sor()
{
  int i;
  SOR *Object;
  UV_VECT *Points;

  Parse_Begin();

  if ((Object = (SOR *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *)Object);
  }

  Object = Create_Sor();

  /* Get number of points. */

  Object->Number = (int)Parse_Float();

  if (Object->Number <4)
  {
    Error("Surface of revolution must have at least four points.");
  }

  /* Get temporary points describing the rotated curve. */

  Points = (UV_VECT *)POV_MALLOC(Object->Number*sizeof(UV_VECT), "temporary surface of revolution points");

  /* Read points (x : radius; y : height; z : not used). */

  for (i = 0; i < Object->Number; i++)
  {
    Parse_Comma();

    Parse_UV_Vect(Points[i]);

    if ((Points[i][X] < 0.0) ||
        ((i > 1 ) && (i < Object->Number - 1) && (Points[i][Y] <= Points[i-1][Y])))
    {
      Error("Incorrect point in surface of revolution.");
    }
  }

  /* Closed or not closed that's the question. */

  EXPECT
    CASE(OPEN_TOKEN)
      Clear_Flag(Object, CLOSED_FLAG);
      EXIT
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  /* There are Number-3 segments! */

  Object->Number -= 3;

  /* Compute spline segments. */

  Compute_Sor(Object, Points);

  /* Compute bounding box. */

  Compute_Sor_BBox(Object);

  /* Parse object's modifiers. */

  Parse_Object_Mods((OBJECT *)Object);

  /* Destroy temporary points. */

  POV_FREE (Points);

  return ((OBJECT *) Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Lathe
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   OBJECT * -
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Read a lathe primitive.
*
* CHANGES
*
*   Jun 1994 : Creation.
*
******************************************************************************/

static OBJECT *Parse_Lathe()
{
  int i;
  LATHE *Object;
  UV_VECT *Points;

  Parse_Begin();

  if ((Object = (LATHE *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *)Object);
  }

  Object = Create_Lathe();

  /* Determine kind of spline used and aspect ratio. */

  EXPECT
    CASE(LINEAR_SPLINE_TOKEN)
      Object->Spline_Type = LINEAR_SPLINE;
    END_CASE

    CASE(QUADRATIC_SPLINE_TOKEN)
      Object->Spline_Type = QUADRATIC_SPLINE;
    END_CASE

    CASE(CUBIC_SPLINE_TOKEN)
      Object->Spline_Type = CUBIC_SPLINE;
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  /* Get number of points. */

  Object->Number = (int)Parse_Float();

  /* Get temporary points describing the rotated curve. */

  Points = (UV_VECT *)POV_MALLOC(Object->Number*sizeof(UV_VECT), "temporary lathe points");

  /* Read points (x : radius; y : height; z : not used). */

  for (i = 0; i < Object->Number; i++)
  {
    Parse_Comma();

    Parse_UV_Vect(Points[i]);

    if ((i > 0) && (i < Object->Number - 1) && (Points[i][X] < 0.0))
    {
      Error("Incorrect point in lathe.");
    }
  }

  switch (Object->Spline_Type)
  {
    case LINEAR_SPLINE :

      Object->Number--;

      break;

    case QUADRATIC_SPLINE :

      Object->Number -= 2;

      break;

    case CUBIC_SPLINE :

      Object->Number -= 3;

      break;
  }

  if (Object->Number <1)
  {
    Error("Lathe must have at least one segment.");
  }

  /* Compute spline segments. */

  Compute_Lathe(Object, Points);

  /* Compute bounding box. */

  Compute_Lathe_BBox(Object);

  /* Parse object's modifiers. */

  Parse_Object_Mods((OBJECT *)Object);

  /* Destroy temporary points. */

  POV_FREE (Points);

  return((OBJECT *) Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Superellipsoid
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   OBJECT * -
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Read a superellipsoid primitive.
*
* CHANGES
*
*   Oct 1994 : Creation.
*
******************************************************************************/

static OBJECT *Parse_Superellipsoid()
{
  UV_VECT V1;
  SUPERELLIPSOID *Object;

  Parse_Begin();

  if ((Object = (SUPERELLIPSOID *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *)Object);
  }

  Object = Create_Superellipsoid();

  Parse_UV_Vect(V1);

  /* The x component is e, the y component is n. */

  Object->Power[X] = 2.0  / V1[X];
  Object->Power[Y] = V1[X] / V1[Y];
  Object->Power[Z] = 2.0  / V1[Y];

  /* Compute bounding box. */

  Compute_Superellipsoid_BBox(Object);

  /* Parse object's modifiers. */

  Parse_Object_Mods((OBJECT *)Object);

  return((OBJECT *) Object);
}


/*****************************************************************************
*
* FUNCTION
*
*   Parse_Torus
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   OBJECT
*
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Jul 1994 : Creation.
*
******************************************************************************/

static OBJECT *Parse_Torus()
{
  TORUS *Object;

  Parse_Begin();

  if ((Object = (TORUS *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *)Object);
  }

  Object = Create_Torus();

  /* Read in the two radii. */

  Object->R = Parse_Float(); /* Big radius */

  Parse_Comma();

  Object->r = Parse_Float(); /* Little radius */

  Compute_Torus_BBox(Object);

  Parse_Object_Mods ((OBJECT *)Object);

  return ((OBJECT *) Object);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Mesh_Texture
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   OBJECT
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Read an individual triangle mesh texture.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

static TEXTURE *Parse_Mesh_Texture()
{
  TEXTURE *Texture;

  Texture = NULL;

  EXPECT
    CASE(TEXTURE_TOKEN)
      Parse_Begin();

      GET(TEXTURE_ID_TOKEN);

      Texture = (TEXTURE *)Token.Constant_Data;

      Parse_End();
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  return(Texture);
}



/*****************************************************************************
*
* FUNCTION
*
*   Parse_Mesh
*
* INPUT
*   
* OUTPUT
*   
* RETURNS
*
*   OBJECT
*   
* AUTHOR
*
*   Dieter Bayer
*   
* DESCRIPTION
*
*   Read a triangle mesh.
*
* CHANGES
*
*   Feb 1995 : Creation.
*
******************************************************************************/

static OBJECT *Parse_Mesh()
{
  int i;
  int number_of_normals, number_of_textures, number_of_triangles, number_of_vertices;
  int max_normals, max_textures, max_triangles, max_vertices;
  DBL l1, l2, l3;
  VECTOR D1, D2, P1, P2, P3, N1, N2, N3, N;
  SNGL_VECT *Normals, *Vertices;
  TEXTURE **Textures;
  MESH *Object;
  MESH_TRIANGLE *Triangles;

  Parse_Begin();

  if ((Object = (MESH *)Parse_Object_Id()) != NULL)
  {
    return((OBJECT *)Object);
  }

  /* Create object. */

  Object = Create_Mesh();

  /* Allocate temporary normals, textures, triangles and vertices. */

  max_normals = 256;

  max_vertices = 256;

  max_textures = 16;

  max_triangles = 256;

  Normals = (SNGL_VECT *)POV_MALLOC(max_normals*sizeof(SNGL_VECT), "temporary triangle mesh data");

  Textures = (TEXTURE **)POV_MALLOC(max_textures*sizeof(TEXTURE *), "temporary triangle mesh data");

  Triangles = (MESH_TRIANGLE *)POV_MALLOC(max_triangles*sizeof(MESH_TRIANGLE), "temporary triangle mesh data");

  Vertices = (SNGL_VECT *)POV_MALLOC(max_vertices*sizeof(SNGL_VECT), "temporary triangle mesh data");

  /* Read raw triangle file. */

  number_of_normals = 0;

  number_of_textures = 0;

  number_of_triangles = 0;

  number_of_vertices = 0;

  /* Create hash tables. */

  Create_Mesh_Hash_Tables();

  EXPECT
    CASE(TRIANGLE_TOKEN)
      Parse_Begin();

      Parse_Vector(P1);  Parse_Comma();
      Parse_Vector(P2);  Parse_Comma();
      Parse_Vector(P3);

      if (!Mesh_Degenerate(P1, P2, P3))
      {
        if (number_of_triangles >= max_triangles)
        {
          if (max_triangles >= INT_MAX/2)
          {
            Error("Too many triangles in triangle mesh.\n");
          }

          max_triangles *= 2;

          Triangles = (MESH_TRIANGLE *)POV_REALLOC(Triangles, max_triangles*sizeof(MESH_TRIANGLE), "triangle triangle mesh data");
        }

        /* Init triangle. */

        Init_Mesh_Triangle(&Triangles[number_of_triangles]);

        Triangles[number_of_triangles].P1 = Mesh_Hash_Vertex(&number_of_vertices, &max_vertices, &Vertices, P1);
        Triangles[number_of_triangles].P2 = Mesh_Hash_Vertex(&number_of_vertices, &max_vertices, &Vertices, P2);
        Triangles[number_of_triangles].P3 = Mesh_Hash_Vertex(&number_of_vertices, &max_vertices, &Vertices, P3);

        Compute_Mesh_Triangle(&Triangles[number_of_triangles], FALSE, P1, P2, P3, N);

        Triangles[number_of_triangles].Normal_Ind = Mesh_Hash_Normal(&number_of_normals, &max_normals, &Normals, N);

        Triangles[number_of_triangles].Texture = Mesh_Hash_Texture(&number_of_textures, &max_textures, &Textures, Parse_Mesh_Texture());

        number_of_triangles++;
      }

      Parse_End();
    END_CASE

    CASE(SMOOTH_TRIANGLE_TOKEN)
      Parse_Begin();

      Parse_Vector(P1);  Parse_Comma();
      Parse_Vector(N1);  Parse_Comma();

      Parse_Vector(P2);  Parse_Comma();
      Parse_Vector(N2);  Parse_Comma();

      Parse_Vector(P3);  Parse_Comma();
      Parse_Vector(N3);

      VLength(l1, N1);
      VLength(l2, N2);
      VLength(l3, N3);

      if ((l1 != 0.0) && (l2 != 0.0) && (l3 != 0.0) && (!Mesh_Degenerate(P1, P2, P3)))
      {
        if (number_of_triangles >= max_triangles)
        {
          if (max_triangles >= INT_MAX/2)
          {
            Error("Too many triangles in triangle mesh.\n");
          }

          max_triangles *= 2;

          Triangles = (MESH_TRIANGLE *)POV_REALLOC(Triangles, max_triangles*sizeof(MESH_TRIANGLE), "triangle triangle mesh data");
        }

        VInverseScaleEq(N1, l1);
        VInverseScaleEq(N2, l2);
        VInverseScaleEq(N3, l3);

        /* Init triangle. */

        Init_Mesh_Triangle(&Triangles[number_of_triangles]);

        Triangles[number_of_triangles].P1 = Mesh_Hash_Vertex(&number_of_vertices, &max_vertices, &Vertices, P1);
        Triangles[number_of_triangles].P2 = Mesh_Hash_Vertex(&number_of_vertices, &max_vertices, &Vertices, P2);
        Triangles[number_of_triangles].P3 = Mesh_Hash_Vertex(&number_of_vertices, &max_vertices, &Vertices, P3);

        /* Check for equal normals. */

        VSub(D1, N1, N2);
        VSub(D2, N1, N3);

        VDot(l1, D1, D1);
        VDot(l2, D2, D2);

        if ((fabs(l1) > EPSILON) || (fabs(l2) > EPSILON))
        {
          /* Smooth triangle. */

          Triangles[number_of_triangles].N1 = Mesh_Hash_Normal(&number_of_normals, &max_normals, &Normals, N1);
          Triangles[number_of_triangles].N2 = Mesh_Hash_Normal(&number_of_normals, &max_normals, &Normals, N2);
          Triangles[number_of_triangles].N3 = Mesh_Hash_Normal(&number_of_normals, &max_normals, &Normals, N3);

          Compute_Mesh_Triangle(&Triangles[number_of_triangles], TRUE, P1, P2, P3, N);
        }
        else
        {
          /* Flat triangle. */

          Compute_Mesh_Triangle(&Triangles[number_of_triangles], FALSE, P1, P2, P3, N);
        }

        Triangles[number_of_triangles].Normal_Ind = Mesh_Hash_Normal(&number_of_normals, &max_normals, &Normals, N);

        Triangles[number_of_triangles].Texture = Mesh_Hash_Texture(&number_of_textures, &max_textures, &Textures, Parse_Mesh_Texture());

        number_of_triangles++;
      }

      Parse_End();
    END_CASE

    OTHERWISE
      UNGET
      EXIT
    END_CASE
  END_EXPECT

  /* Destroy hash tables. */

  Destroy_Mesh_Hash_Tables();

  /* If there are no triangles something went wrong. */

  if (number_of_triangles == 0)
  {
    Error("No triangles in triangle mesh.\n");
  }

  /* Init triangle mesh data. */

  Object->Data = POV_MALLOC(sizeof(MESH_DATA), "triangle mesh data");

  Object->Data->References = 1;

  Object->Data->Tree = NULL;
  
  Object->Data->Normals   = NULL;
  Object->Data->Textures  = NULL;
  Object->Data->Triangles = NULL;
  Object->Data->Vertices  = NULL;

  /* Allocate memory for normals, textures, triangles and vertices. */

  Object->Data->Number_Of_Normals = number_of_normals;

  Object->Data->Number_Of_Textures = number_of_textures;

  Object->Data->Number_Of_Triangles = number_of_triangles;

  Object->Data->Number_Of_Vertices = number_of_vertices;

  Object->Data->Normals = (SNGL_VECT *)POV_MALLOC(number_of_normals*sizeof(SNGL_VECT), "triangle mesh data");

  if (number_of_textures)
  {
    Set_Flag(Object, MULTITEXTURE_FLAG);

    Object->Data->Textures = (TEXTURE **)POV_MALLOC(number_of_textures*sizeof(TEXTURE *), "triangle mesh data");
  }

  Object->Data->Triangles = (MESH_TRIANGLE *)POV_MALLOC(number_of_triangles*sizeof(MESH_TRIANGLE), "triangle mesh data");

  Object->Data->Vertices = (SNGL_VECT *)POV_MALLOC(number_of_vertices*sizeof(SNGL_VECT), "triangle mesh data");

  /* Copy normals, textures, triangles and vertices into mesh. */

  for (i = 0; i < number_of_normals; i++)
  {
    Assign_SNGL_Vect(Object->Data->Normals[i], Normals[i]);
  }

  for (i = 0; i < number_of_textures; i++)
  {
    Object->Data->Textures[i] = Textures[i];
  }

  for (i = 0; i < number_of_triangles; i++)
  {
    Object->Data->Triangles[i] = Triangles[i];
  }

  for (i = 0; i < number_of_vertices; i++)
  {
    Assign_SNGL_Vect(Object->Data->Vertices[i], Vertices[i]);
  }

  /* Free temporary memory. */

  POV_FREE(Normals);
  POV_FREE(Textures);
  POV_FREE(Triangles);
  POV_FREE(Vertices);

/*
  Render_Info("Mesh: %ld bytes: %ld vertices, %ld normals, %ld textures, %ld triangles\n",
    Object->Data->Number_Of_Normals*sizeof(SNGL_VECT)+
    Object->Data->Number_Of_Textures*sizeof(TEXTURE *)+
    Object->Data->Number_Of_Triangles*sizeof(MESH_TRIANGLE)+
    Object->Data->Number_Of_Vertices*sizeof(SNGL_VECT),
    Object->Data->Number_Of_Vertices,
    Object->Data->Number_Of_Normals,
    Object->Data->Number_Of_Textures,
    Object->Data->Number_Of_Triangles);
*/

  /* Create bounding box. */

  Compute_Mesh_BBox(Object);

  /* Parse object modifiers. */

  Parse_Object_Mods((OBJECT *)Object);

  /* Create bounding box tree. */

  Build_Mesh_BBox_Tree(Object);

  return((OBJECT *)Object);
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

static
OBJECT *Parse_Poly (order)
  int order;
  {
   POLY *Object;

   Parse_Begin ();

   if ( (Object = (POLY *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   if (order == 0)
     {
      order = (int)Parse_Float();      Parse_Comma();
      if (order < 2 || order > MAX_ORDER)
        Error("Order of poly is out of range.");
     }

   Object = Create_Poly(order);

   Parse_Coeffs(Object->Order, &(Object->Coeffs[0]));

   Compute_Poly_BBox(Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_Bicubic_Patch ()
  {
   BICUBIC_PATCH *Object;
   int i, j;

   Parse_Begin ();

   if ( (Object = (BICUBIC_PATCH *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   Object = Create_Bicubic_Patch();

   EXPECT
     CASE_FLOAT
       Warn(1.5, "Should use keywords for bicubic parameters.");
       Object->Patch_Type = (int)Parse_Float();
       if (Object->Patch_Type == 2 ||
           Object->Patch_Type == 3)
           Object->Flatness_Value = Parse_Float();
         else
           Object->Flatness_Value = 0.1;
       Object->U_Steps = (int)Parse_Float();
       Object->V_Steps = (int)Parse_Float();
       EXIT
     END_CASE
       
     CASE (TYPE_TOKEN)
       Object->Patch_Type = (int)Parse_Float();
     END_CASE

     CASE (FLATNESS_TOKEN)
       Object->Flatness_Value = Parse_Float();
     END_CASE

     CASE (V_STEPS_TOKEN)
       Object->V_Steps = (int)Parse_Float();
     END_CASE

     CASE (U_STEPS_TOKEN)
       Object->U_Steps = (int)Parse_Float();
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   if (Object->Patch_Type > 1)
     {
      Object->Patch_Type = 1;
      Warn(0.0, "Patch type no longer supported. Using type 1.");
     }

   if ((Object->Patch_Type < 0) || (Object->Patch_Type > MAX_PATCH_TYPE))
     Error("Undefined bicubic patch type.");

   Parse_Comma();
   for (i=0;i<4;i++)
     for (j=0;j<4;j++)
       {
                  Parse_Vector(Object->Control_Points[i][j]);
        if (!((i==3)&&(j==3)))
          Parse_Comma();
       };
   Precompute_Patch_Values(Object); /* interpolated mesh coords */

   Compute_Bicubic_Patch_BBox(Object);  

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_TrueType ()
  {
   OBJECT *Object;
   char *filename, *text_string;
   DBL depth;
   VECTOR offset;

   Parse_Begin ();
   
   GET(TTF_TOKEN);

   if ( (Object = (OBJECT *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = (OBJECT *)Create_CSG_Union ();
   /*** Object = Create_TTF(); */

   /* Parse the TrueType font file name */
   filename = Parse_String();
   Parse_Comma();

   /* Parse the text string to be rendered */
   text_string = Parse_String();
   Parse_Comma();

   /* Get the extrusion depth */
   depth = Parse_Float(); Parse_Comma ();

   /* Get the offset vector */
   Parse_Vector(offset);

   /* Process all this good info */
   ProcessNewTTF((OBJECT *)Object, filename, text_string, depth, offset);

   /* Free up the filename and text string memory */
   POV_FREE (filename);
   POV_FREE (text_string);

   /**** Compute_TTF_BBox(Object); */
   Compute_CSG_BBox((OBJECT *)Object);

   /* Get any rotate/translate or texturing stuff */
   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_CSG (CSG_Type)
  int CSG_Type;
  {
   CSG *Object;
   OBJECT *Local;
   int Object_Count = 0;

   Parse_Begin ();

   if ( (Object = (CSG *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);

   if (CSG_Type & CSG_UNION_TYPE)
     Object = Create_CSG_Union ();
   else
     if (CSG_Type & CSG_MERGE_TYPE)
       Object = Create_CSG_Merge ();
     else
       Object = Create_CSG_Intersection ();

   Object->Children = NULL;

   while ((Local = Parse_Object ()) != NULL)
     {
      if ((CSG_Type & CSG_INTERSECTION_TYPE) && (Local->Type & PATCH_OBJECT))
        Warn(0.0, "Patch objects not allowed in intersection.");
      Object_Count++;
      if ((CSG_Type & CSG_DIFFERENCE_TYPE) && (Object_Count > 1))
        Invert_Object (Local);
      Object->Type |=  (Local->Type & CHILDREN_FLAGS);
      Local->Type |= IS_CHILD_OBJECT;
      Link(Local, &Local->Sibling, &Object->Children);
     };

   if ((Object_Count < 2) && (opts.Language_Version >= 1.5))
     Warn(1.5, "Should have at least 2 objects in csg.");

   Compute_CSG_BBox((OBJECT *)Object);

   Parse_Object_Mods ((OBJECT *)Object);

   return ((OBJECT *) Object);
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

static
OBJECT *Parse_Light_Source ()
  {
   DBL Len;
   VECTOR Local_Vector;
   MATRIX Local_Matrix;
   TRANSFORM Local_Trans;
   LIGHT_SOURCE *Object;

   Parse_Begin ();

   if ( (Object = (LIGHT_SOURCE *)Parse_Object_Id()) != NULL)
      return ((OBJECT *) Object);
      
   Object = Create_Light_Source ();

   Parse_Vector(Object->Center);

   Parse_Comma();

   Parse_Colour (Object->Colour);

   EXPECT
     CASE (LOOKS_LIKE_TOKEN)
       if (Object->Children != NULL)
         Error("Only one looks_like allowed per light_source.");
       Parse_Begin ();
       Object->Type &= ~(int)PATCH_OBJECT;
       if ((Object->Children = Parse_Object ()) == NULL)
         Parse_Error_Str ("object");
       Compute_Translation_Transform(&Local_Trans, Object->Center);
       Translate_Object (Object->Children, Object->Center, &Local_Trans);
       Parse_Object_Mods (Object->Children);
       Set_Flag(Object->Children, NO_SHADOW_FLAG);
       Set_Flag(Object, NO_SHADOW_FLAG);
       Object->Type |= (Object->Children->Type & CHILDREN_FLAGS);
     END_CASE

     CASE (FILL_LIGHT_TOKEN)
       Object->Light_Type = FILL_LIGHT_SOURCE;
     END_CASE

     CASE (SPOTLIGHT_TOKEN)
       Object->Light_Type = SPOT_SOURCE;
     END_CASE

     CASE (CYLINDER_TOKEN)
       Object->Light_Type = CYLINDER_SOURCE;
     END_CASE

     CASE (POINT_AT_TOKEN)
       if ((Object->Light_Type == SPOT_SOURCE) || (Object->Light_Type == CYLINDER_SOURCE))
         Parse_Vector(Object->Points_At);
       else
         Not_With ("point_at","standard light source");
     END_CASE

     CASE (TIGHTNESS_TOKEN)
       if ((Object->Light_Type == SPOT_SOURCE) || (Object->Light_Type == CYLINDER_SOURCE))
         Object->Coeff = Parse_Float();
       else
         Not_With ("tightness","standard light source");
     END_CASE

     CASE (RADIUS_TOKEN)
       if ((Object->Light_Type == SPOT_SOURCE) || (Object->Light_Type == CYLINDER_SOURCE))
         Object->Radius = Parse_Float();
       else
         Not_With ("radius","standard light source");
     END_CASE

     CASE (FALLOFF_TOKEN)
       if ((Object->Light_Type == SPOT_SOURCE) || (Object->Light_Type == CYLINDER_SOURCE))
         Object->Falloff = Parse_Float();
       else
         Not_With ("falloff","standard light source");
     END_CASE

     CASE (FADE_DISTANCE_TOKEN)
       Object->Fade_Distance = Parse_Float();
     END_CASE

     CASE (FADE_POWER_TOKEN)
       Object->Fade_Power = Parse_Float();
     END_CASE

     CASE (AREA_LIGHT_TOKEN)
       Object->Area_Light = TRUE;
       Parse_Vector (Object->Axis1); Parse_Comma ();
       Parse_Vector (Object->Axis2); Parse_Comma ();
       Object->Area_Size1 = (int)Parse_Float(); Parse_Comma ();
       Object->Area_Size2 = (int)Parse_Float();
       Object->Light_Grid = Create_Light_Grid (Object->Area_Size1, Object->Area_Size2);
     END_CASE

     CASE (JITTER_TOKEN)
       Object->Jitter = TRUE;
     END_CASE

     CASE (TRACK_TOKEN)
       Object->Track = TRUE;
     END_CASE

     CASE (ADAPTIVE_TOKEN)
       Object->Adaptive_Level = (int)Parse_Float();
     END_CASE

     CASE (ATMOSPHERIC_ATTENUATION_TOKEN)
       Experimental_Flag |= EF_ATMOS;
       Object->Atmospheric_Attenuation = Allow_Float(1.0) > 0.0;
     END_CASE

     CASE (ATMOSPHERE_TOKEN)
       Experimental_Flag |= EF_ATMOS;
       Object->Atmosphere_Interaction = Allow_Float(1.0) > 0.0;
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Translation_Transform(&Local_Trans, Local_Vector);
       Translate_Object ((OBJECT *)Object, Local_Vector, &Local_Trans);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Rotation_Transform(&Local_Trans, Local_Vector);
       Rotate_Object ((OBJECT *)Object, Local_Vector, &Local_Trans);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (Local_Vector);
       Compute_Scaling_Transform(&Local_Trans, Local_Vector);
       Scale_Object ((OBJECT *)Object, Local_Vector, &Local_Trans);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Object ((OBJECT *)Object, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     CASE (MATRIX_TOKEN)
       Parse_Matrix (Local_Matrix);
       Compute_Matrix_Transform(&Local_Trans, Local_Matrix);
       Transform_Object ((OBJECT *)Object, &Local_Trans);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Parse_End ();

   if (Object->Light_Type == SPOT_SOURCE)
   {
     Object->Radius  = cos(Object->Radius * M_PI_180);
     Object->Falloff = cos(Object->Falloff * M_PI_180);
   }

   VSub(Object->Direction, Object->Points_At, Object->Center);

   VLength(Len, Object->Direction);

   if (Len > EPSILON)
   {
     VInverseScaleEq(Object->Direction, Len);
   }

   return ((OBJECT *)Object);
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

OBJECT *Parse_Object ()
  {
   OBJECT *Object = NULL;

   EXPECT

     CASE (JULIA_FRACTAL_TOKEN)
       Object = Parse_Julia_Fractal ();
       EXIT
     END_CASE

     CASE (SPHERE_TOKEN)
       Object = Parse_Sphere ();
       EXIT
     END_CASE

     CASE (PLANE_TOKEN)
       Object = Parse_Plane ();
       EXIT
     END_CASE

     CASE (CONE_TOKEN)
       Object = Parse_Cone ();
       EXIT
     END_CASE

     CASE (CYLINDER_TOKEN)
       Object = Parse_Cylinder ();
       EXIT
     END_CASE

     CASE (DISC_TOKEN)
       Object = Parse_Disc ();
       EXIT
     END_CASE

     CASE (QUADRIC_TOKEN)
       Object = Parse_Quadric ();
       EXIT
     END_CASE

     CASE (CUBIC_TOKEN)
       Object = Parse_Poly (3);
       EXIT
     END_CASE

     CASE (QUARTIC_TOKEN)
       Object = Parse_Poly (4);
       EXIT
     END_CASE

     CASE (POLY_TOKEN)
       Object = Parse_Poly (0);
       EXIT
     END_CASE

     CASE (TORUS_TOKEN)
       Object = Parse_Torus ();
       EXIT
     END_CASE

     /* Parse lathe primitive. [DB 8/94] */

     CASE (LATHE_TOKEN)
       Object = Parse_Lathe();
       EXIT
     END_CASE

     /* Parse polygon primitive. [DB 8/94] */

     CASE (POLYGON_TOKEN)
       Object = Parse_Polygon();
       EXIT
     END_CASE

     /* Parse prism primitive. [DB 8/94] */

     CASE (PRISM_TOKEN)
       Object = Parse_Prism();
       EXIT
     END_CASE

     /* Parse surface of revolution primitive. [DB 8/94] */

     CASE (SOR_TOKEN)
       Object = Parse_Sor();
       EXIT
     END_CASE

     /* Parse superellipsoid primitive. [DB 11/94] */

     CASE (SUPERELLIPSOID_TOKEN)
       Object = Parse_Superellipsoid();
       EXIT
     END_CASE

     /* Parse triangle mesh primitive. [DB 2/95] */

     CASE (MESH_TOKEN)
       Object = Parse_Mesh();
       EXIT
     END_CASE

     CASE (TEXT_TOKEN)
       Object = Parse_TrueType ();
       EXIT
     END_CASE

     CASE (OBJECT_ID_TOKEN)
       Object = Copy_Object((OBJECT *) Token.Constant_Data);
       EXIT
     END_CASE

     CASE (UNION_TOKEN)
       Object = Parse_CSG (CSG_UNION_TYPE);
       EXIT
     END_CASE

     CASE (COMPOSITE_TOKEN)
       Warn(1.5, "Use union instead of composite.");
       Object = Parse_CSG (CSG_UNION_TYPE);
       EXIT
     END_CASE

     CASE (MERGE_TOKEN)
       Object = Parse_CSG (CSG_MERGE_TYPE);
       EXIT
     END_CASE

     CASE (INTERSECTION_TOKEN)
       Object = Parse_CSG (CSG_INTERSECTION_TYPE);
       EXIT
     END_CASE

     CASE (DIFFERENCE_TOKEN)
       Object = Parse_CSG (CSG_DIFFERENCE_TYPE+CSG_INTERSECTION_TYPE);
       EXIT
     END_CASE

     CASE (BICUBIC_PATCH_TOKEN)
       Object = Parse_Bicubic_Patch ();
       EXIT
     END_CASE

     CASE (TRIANGLE_TOKEN)
       Object = Parse_Triangle ();
       EXIT
     END_CASE

     CASE (SMOOTH_TRIANGLE_TOKEN)
       Object = Parse_Smooth_Triangle ();
       EXIT
     END_CASE

     CASE (HEIGHT_FIELD_TOKEN)
       Object = Parse_HField ();
       EXIT
     END_CASE

     CASE (BOX_TOKEN)
       Object = Parse_Box ();
       EXIT
     END_CASE

     CASE (BLOB_TOKEN)
       Object = Parse_Blob ();
       EXIT
     END_CASE

     CASE (LIGHT_SOURCE_TOKEN)
       Object = Parse_Light_Source ();
       EXIT
     END_CASE

     CASE (OBJECT_TOKEN)
       Parse_Begin ();
       Object = Parse_Object ();
       if (!Object)
         Parse_Error_Str ("object");
       Parse_Object_Mods ((OBJECT *)Object);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   return ((OBJECT *) Object);
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

void Parse_Default ()
  {
   TEXTURE *Local_Texture;
   PIGMENT *Local_Pigment;
   TNORMAL *Local_Tnormal;
   FINISH  *Local_Finish;
   HALO    *Local_Halo;

   Not_In_Default = FALSE;
   Parse_Begin();
   
   EXPECT
     CASE (TEXTURE_TOKEN)
       Local_Texture = Default_Texture;
       Parse_Begin ();
       Default_Texture = Parse_Texture();
       Parse_End ();
       if (Default_Texture->Type != PLAIN_PATTERN)
         Error("Default texture cannot be material map or tiles.");
       if (Default_Texture->Next != NULL)
         Error("Default texture cannot be layered.");
       Destroy_Textures(Local_Texture);
     END_CASE

     CASE (PIGMENT_TOKEN)
       Local_Pigment = Copy_Pigment((Default_Texture->Pigment));
       Parse_Begin ();
       Parse_Pigment (&Local_Pigment);
       Parse_End ();
       Destroy_Pigment(Default_Texture->Pigment);
       Default_Texture->Pigment = Local_Pigment;
     END_CASE

     CASE (TNORMAL_TOKEN)
       Local_Tnormal = Copy_Tnormal((Default_Texture->Tnormal));
       Parse_Begin ();
       Parse_Tnormal (&Local_Tnormal);
       Parse_End ();
       Destroy_Tnormal(Default_Texture->Tnormal);
       Default_Texture->Tnormal = Local_Tnormal;
     END_CASE

     CASE (FINISH_TOKEN)
       Local_Finish = Copy_Finish((Default_Texture->Finish));
       Parse_Finish (&Local_Finish);
       Destroy_Finish(Default_Texture->Finish);
       Default_Texture->Finish = Local_Finish;
     END_CASE

     CASE (HALO_TOKEN)
       Local_Halo = Copy_Halo((Default_Texture->Halo));
       Parse_Halo (&Local_Halo);
       Destroy_Halo(Default_Texture->Halo);
       Default_Texture->Halo = Local_Halo;
     END_CASE

     CASE (CAMERA_TOKEN)
       Parse_Camera (&Default_Camera);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Parse_End();

   Not_In_Default = TRUE;
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

static void Parse_Frame ()
  {
   OBJECT *Object;
   RAINBOW  *Local_Rainbow;
   FOG  *Local_Fog;
   SKYSPHERE  *Local_Skysphere;
   ATMOSPHERE *Local_Atmosphere;

   EXPECT
     CASE (RAINBOW_TOKEN)
       Local_Rainbow = Parse_Rainbow();
       Local_Rainbow->Next = Frame.Rainbow;
       Frame.Rainbow = Local_Rainbow;
     END_CASE

     CASE (SKYSPHERE_TOKEN)
       Local_Skysphere = Parse_Skysphere();
       if (Frame.Skysphere != NULL)
       {
         Warn(0.0, "Only one sky-sphere allowed (last one will be used).");
         Destroy_Skysphere(Frame.Skysphere);
       }
       Frame.Skysphere = Local_Skysphere;
     END_CASE

     CASE (FOG_TOKEN)
       Local_Fog = Parse_Fog();
       Local_Fog->Next = Frame.Fog;
       Frame.Fog = Local_Fog;
     END_CASE

     CASE (ATMOSPHERE_TOKEN)
       Local_Atmosphere = Parse_Atmosphere();
       if (Frame.Atmosphere != NULL)
       {
         Warn(0.0, "Only one atmosphere allowed (last one will be used).");
         Destroy_Atmosphere(Frame.Atmosphere);
       }
       Frame.Atmosphere = Local_Atmosphere;
     END_CASE

     CASE (BACKGROUND_TOKEN)
       Parse_Begin();
       Parse_Colour (Frame.Background_Colour);
       Parse_End();
     END_CASE

     CASE (CAMERA_TOKEN)
       Parse_Camera (&Frame.Camera);
     END_CASE

     CASE (DECLARE_TOKEN)
       UNGET
       Warn(2.99,"Should have '#' before 'declare'.");
       Parse_Directive (FALSE);
     END_CASE

     CASE (INCLUDE_TOKEN)
       UNGET
       Warn(2.99,"Should have '#' before 'include'.");
       Parse_Directive (FALSE);
     END_CASE

     CASE (FLOAT_FUNCT_TOKEN)
       switch(Token.Function_Id)
         {
          case VERSION_TOKEN:
            UNGET
            Parse_Directive (FALSE);
            UNGET
            break;
            
          default:
            UNGET
            Parse_Error_Str ("object or directive");
            break;
         }
     END_CASE

     CASE (MAX_TRACE_LEVEL_TOKEN)
       Global_Setting_Warn();
       Max_Trace_Level = (int) Parse_Float ();
     END_CASE

     CASE (MAX_INTERSECTIONS)
       Global_Setting_Warn();
       Max_Intersections = (int)Parse_Float ();
     END_CASE

     CASE (DEFAULT_TOKEN)
       Parse_Default();
     END_CASE

     CASE (END_OF_FILE_TOKEN)
       EXIT
     END_CASE

     CASE (GLOBAL_SETTINGS_TOKEN)
       Parse_Global_Settings();
     END_CASE

     OTHERWISE
       UNGET
       Object = Parse_Object();
       if (Object == NULL)
         Parse_Error_Str ("object or directive");
       Post_Process (Object, NULL);
       Link_To_Frame (Object);
     END_CASE
   END_EXPECT
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
*   Mar 1996 : Add line number info to warning message  [AED]
*
******************************************************************************/

static void Global_Setting_Warn()
{
  if (opts.Language_Version >= 3.0)
  {
    Warning(0.0, "%s:%d: warning: '%s' should be in 'global_settings{...}' statement.\n",
            Token.Filename, Token.Token_Line_No+1, Token.Token_String);
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

static void Parse_Global_Settings()
{
   Parse_Begin();
   EXPECT
     CASE (IRID_WAVELENGTH_TOKEN)
       Parse_Colour (Frame.Irid_Wavelengths);
     END_CASE

     CASE (ASSUMED_GAMMA_TOKEN)
     {
       DBL AssumedGamma;
       AssumedGamma = Parse_Float ();

       if (fabs(AssumedGamma - opts.DisplayGamma) < 0.1)
       {
         opts.GammaFactor = 1.0;
         opts.Options &= ~GAMMA_CORRECT; /* turn off gamma correction */
       }
       else
       {
         opts.GammaFactor = AssumedGamma/opts.DisplayGamma;
         opts.Options |= GAMMA_CORRECT; /* turn on gamma correction */
       }
     }
     END_CASE

     CASE (MAX_TRACE_LEVEL_TOKEN)
       Max_Trace_Level = (int) Parse_Float ();
     END_CASE

     CASE (ADC_BAILOUT_TOKEN)
       ADC_Bailout = Parse_Float ();
     END_CASE

     CASE (NUMBER_OF_WAVES_TOKEN)
       Number_Of_Waves = (int) Parse_Float ();
       if(Number_Of_Waves <=0)
       {
         Warn(0.0, "Illegal Value: Number_Of_Waves = 0. Changed to 1.");
         Number_Of_Waves = 1;
       }
     END_CASE

     CASE (MAX_INTERSECTIONS)
       Max_Intersections = (int)Parse_Float ();
     END_CASE

     CASE (AMBIENT_LIGHT_TOKEN)
       Parse_Colour (Frame.Ambient_Light);
     END_CASE

     CASE (RADIOSITY_TOKEN)
       Experimental_Flag |= EF_RADIOS;
       Parse_Begin();
       EXPECT
         CASE (BRIGHTNESS_TOKEN)
           if ((opts.Radiosity_Brightness = Parse_Float()) <= 0.0)
           {
              Error("Radiosity brightness must be a positive number.");
           }
         END_CASE

         CASE (COUNT_TOKEN)
           if (( opts.Radiosity_Count = (int)Parse_Float()) <= 0)
           {
             Error("Radiosity count must be a positive number.");
           }
           if ( opts.Radiosity_Count > 1600)
           {
             Error("Radiosity count can not be more than 1600.");
             opts.Radiosity_Count = 1600;
           }
         END_CASE

         CASE (DISTANCE_MAXIMUM_TOKEN)
           if (( opts.Radiosity_Dist_Max = Parse_Float()) < 0.0)
           {
             Error("Radiosity distance maximum must be a positive number.");
           }
         END_CASE

         CASE (ERROR_BOUND_TOKEN)
           if (( opts.Radiosity_Error_Bound = Parse_Float()) <= 0.0)
           {
             Error("Radiosity error bound must be a positive number.");
           }
         END_CASE

         CASE (GRAY_THRESHOLD_TOKEN)
           opts.Radiosity_Gray = Parse_Float();
           if (( opts.Radiosity_Gray < 0.0) || ( opts.Radiosity_Gray > 1.0))
           {
              Error("Radiosity gray threshold must be from 0.0 to 1.0.");
           }
         END_CASE

         CASE (LOW_ERROR_FACTOR_TOKEN)
           if (( opts.Radiosity_Low_Error_Factor = Parse_Float()) <= 0.0)
           {
             Error("Radiosity low error factor must be a positive number.");
           }
         END_CASE

         CASE (MINIMUM_REUSE_TOKEN)
           if (( opts.Radiosity_Min_Reuse = Parse_Float()) < 0.0)
           {
              Error("Radiosity min reuse can not be a negative number.");
           }
         END_CASE

         CASE (NEAREST_COUNT_TOKEN)
           opts.Radiosity_Nearest_Count = (int)Parse_Float();
           if (( opts.Radiosity_Nearest_Count < 1) ||
               ( opts.Radiosity_Nearest_Count > MAX_NEAREST_COUNT))
           {
              Error("Radiosity nearest count must be a value from 1 to %ld.", (long)MAX_NEAREST_COUNT);
           }
         END_CASE

         CASE (RECURSION_LIMIT_TOKEN)
           if (( opts.Radiosity_Recursion_Limit = (int)Parse_Float()) <= 0)
           {
              Error("Radiosity recursion limit must be a positive number.");
           }
         END_CASE

         OTHERWISE
           UNGET
           EXIT
         END_CASE
       END_EXPECT
       Parse_End();
     END_CASE
 
     CASE (HF_GRAY_16_TOKEN)
       if (Allow_Float(1.0)>EPSILON)     
       {
         opts.Options |= HF_GRAY_16;
         opts.PaletteOption = GREY;        /* Force gray scale preview */
         Output_File_Handle->file_type = HF_FTYPE;
       }
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT
   Parse_End();
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

void Destroy_Frame()
{
    FOG *Fog, *Next_Fog;
    RAINBOW *Rainbow, *Next_Rainbow;

    Destroy_Camera (Frame.Camera); Frame.Camera=NULL;

    /* Destroy fogs. [DB 12/94] */

    for (Fog = Frame.Fog; Fog != NULL;)
    {
      Next_Fog = Fog->Next;

      Destroy_Fog(Fog);

      Fog = Next_Fog;
    }
    
    Frame.Fog = NULL;

    /* Destroy rainbows. [DB 12/94] */

    for (Rainbow = Frame.Rainbow; Rainbow != NULL;)
    {
      Next_Rainbow = Rainbow->Next;

      Destroy_Rainbow(Rainbow);

      Rainbow = Next_Rainbow;
    }

    Frame.Rainbow = NULL;

    /* Destroy skysphere. [DB 12/94] */

    Destroy_Skysphere(Frame.Skysphere);

    Frame.Skysphere = NULL;

    /* Destroy atmosphere. [DB 1/95] */

    Destroy_Atmosphere(Frame.Atmosphere);

    Frame.Atmosphere = NULL;

    if (Frame.Objects != NULL) {
       Destroy_Object (Frame.Objects);
       Frame.Objects = NULL;
       Frame.Light_Sources = NULL;
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

static void Parse_Camera (Camera_Ptr)
CAMERA **Camera_Ptr;
{
   int i;
   DBL Direction_Length = 1.0, Up_Length, Right_Length, Handedness;
   DBL k1, k2, k3;
   VECTOR Local_Vector;
   MATRIX Local_Matrix;
   TRANSFORM Local_Trans;
   CAMERA *New;

   Parse_Begin ();

   EXPECT
     CASE (CAMERA_ID_TOKEN)
       Destroy_Camera(*Camera_Ptr);
       *Camera_Ptr = Copy_Camera ((CAMERA *) Token.Constant_Data);
       EXIT
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   New = *Camera_Ptr;

   EXPECT
     /* Get camera type. [DB 7/94] */

     CASE (PERSPECTIVE_TOKEN)
       New->Type = PERSPECTIVE_CAMERA;
     END_CASE

     CASE (ORTHOGRAPHIC_TOKEN)
       New->Type = ORTHOGRAPHIC_CAMERA;
       /* 
        * Resize right and up vector to get the same image 
        * area as we get with the perspective camera. 
        */
       VSub(Local_Vector, New->Look_At, New->Location);
       VLength(k1, Local_Vector);
       VLength(k2, New->Direction);
       if ((k1 > EPSILON) && (k2 > EPSILON))
       {
         VScaleEq(New->Right, k1 / k2);
         VScaleEq(New->Up, k1 / k2);
       }
     END_CASE

     CASE (FISHEYE_TOKEN)
       New->Type = FISHEYE_CAMERA;
     END_CASE

     CASE (ULTRA_WIDE_ANGLE_TOKEN)
       New->Type = ULTRA_WIDE_ANGLE_CAMERA;
     END_CASE

     CASE (OMNIMAX_TOKEN)
       New->Type = OMNIMAX_CAMERA;
     END_CASE

     CASE (PANORAMIC_TOKEN)
       New->Type = PANORAMIC_CAMERA;
     END_CASE

     CASE (CYLINDER_TOKEN)
       i = (int)Parse_Float();
       switch (i)
       {
         case 1: New->Type = CYL_1_CAMERA; break;
         case 2: New->Type = CYL_2_CAMERA; break;
         case 3: New->Type = CYL_3_CAMERA; break;
         case 4: New->Type = CYL_4_CAMERA; break;
       }       
     END_CASE

     CASE (TEST_CAMERA_1_TOKEN)
       New->Type = TEST_CAMERA_1;
     END_CASE

     CASE (TEST_CAMERA_2_TOKEN)
       New->Type = TEST_CAMERA_2;
     END_CASE

     CASE (TEST_CAMERA_3_TOKEN)
       New->Type = TEST_CAMERA_3;
     END_CASE

     CASE (TEST_CAMERA_4_TOKEN)
       New->Type = TEST_CAMERA_4;
     END_CASE

     /* Read viewing angle. Scale direction vector if necessary. [DB 7/94] */

     CASE (ANGLE_TOKEN)
       New->Angle = Parse_Float();

       if (New->Angle < 0.0)
       {
         Error("Negative viewing angle.");
       }

       if (New->Type == PERSPECTIVE_CAMERA)
       {
         if (New->Angle >= 180.0)
         {
           Error("Viewing angle has to be smaller than 180 degrees.");
         }

         VNormalize(New->Direction, New->Direction);

         VLength (Right_Length, New->Right);

         Direction_Length = Right_Length / tan(New->Angle * M_PI_360)/2.0;

         VScaleEq(New->Direction, Direction_Length);
       }
     END_CASE


     /* Read primary ray pertubation. [DB 7/94] */

     CASE (TNORMAL_TOKEN)
       Parse_Begin ();
       Parse_Tnormal(&(New->Tnormal));
       Parse_End ();
     END_CASE

     CASE (LOCATION_TOKEN)
       Parse_Vector(New->Location);
     END_CASE

     CASE (DIRECTION_TOKEN)
       Parse_Vector(New->Direction);
     END_CASE

     CASE (UP_TOKEN)
       Parse_Vector(New->Up);
     END_CASE

     CASE (RIGHT_TOKEN)
       Parse_Vector(New->Right);
     END_CASE

     CASE (SKY_TOKEN)
       Parse_Vector(New->Sky);
     END_CASE

     CASE (LOOK_AT_TOKEN)
       VLength (Direction_Length, New->Direction);
       VLength (Up_Length,        New->Up);
       VLength (Right_Length,     New->Right);
       VCross  (Local_Vector,     New->Up,        New->Direction);
       VDot    (Handedness,       Local_Vector,   New->Right);

       Parse_Vector (New->Direction);
       Assign_Vector(New->Look_At, New->Direction);

       VSub          (New->Direction, New->Direction, New->Location);

       /* Check for zero length direction vector. */

       if (VSumSqr(New->Direction) < EPSILON)
       {
         Error("Camera location and look_at point must be different.\n");
       }
       
       VNormalize    (New->Direction, New->Direction);

       /* Save Right vector */

       Assign_Vector (Local_Vector, New->Right);

       VCross        (New->Right, New->Sky, New->Direction);

       /* Avoid DOMAIN error (from Terry Kanakis) */

       if((fabs(New->Right[X]) < EPSILON) &&
          (fabs(New->Right[Y]) < EPSILON) &&
          (fabs(New->Right[Z]) < EPSILON))
       {
         /* Restore Right vector*/

         Assign_Vector (New->Right, Local_Vector);
       }

       VNormalize (New->Right,     New->Right);
       VCross     (New->Up,        New->Direction, New->Right);
       VScale     (New->Direction, New->Direction, Direction_Length);

       if (Handedness > 0.0)
         VScaleEq (New->Right, Right_Length)
       else
         VScaleEq (New->Right, -Right_Length);

       VScaleEq(New->Up, Up_Length);
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (Local_Vector);
       Translate_Camera (New, Local_Vector);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (Local_Vector);
       Rotate_Camera (New, Local_Vector);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (Local_Vector);
       Scale_Camera (New, Local_Vector);
     END_CASE

     CASE (TRANSFORM_TOKEN)
       GET(TRANSFORM_ID_TOKEN)
       Transform_Camera (New, (TRANSFORM *)Token.Constant_Data);
     END_CASE

     CASE (MATRIX_TOKEN)
       Parse_Matrix (Local_Matrix);
       Compute_Matrix_Transform(&Local_Trans, Local_Matrix);
       Transform_Camera (New, &Local_Trans);
     END_CASE

     /* Parse focal blur stuff. */

     CASE (BLUR_SAMPLES_TOKEN)
        New->Blur_Samples = Parse_Float();
        if (New->Blur_Samples <= 0)
        {
          Error("Illegal number of focal blur samples.\n");
        }
     END_CASE

     CASE (CONFIDENCE_TOKEN)
        k1 = Parse_Float();
        if ((k1 > 0.0) && (k1 < 1.0))
        {
          New->Confidence = k1;
        }
        else
        {
          Warn(0.0, "Illegal confidence value. Default is used.");
        }
     END_CASE

     CASE (VARIANCE_TOKEN)
        k1 = Parse_Float();
        if ((k1 >= 0.0) && (k1 <= 1.0))
        {
          New->Variance = k1;
        }
        else
        {
          Warn(0.0, "Illegal variance value. Default is used.");
        }
     END_CASE

     CASE (APERTURE_TOKEN)
        New->Aperture = Parse_Float();
     END_CASE

     CASE (FOCAL_POINT_TOKEN)
        Parse_Vector(Local_Vector);
        VSubEq(Local_Vector, New->Location);
        VLength (New->Focal_Distance, Local_Vector);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT
   Parse_End ();

   /* Make sure the focal distance hasn't been explicitly given */
   if ( New->Focal_Distance < 0.0 )
      New->Focal_Distance = Direction_Length;
   if ( New->Focal_Distance == 0.0 )
      New->Focal_Distance = 1.0;

   /* Print a warning message if vectors are not perpendicular. [DB 10/94] */

   VDot(k1, New->Right, New->Up);
   VDot(k2, New->Right, New->Direction);
   VDot(k3, New->Up, New->Direction);

   if ((fabs(k1) > EPSILON) || (fabs(k2) > EPSILON) || (fabs(k3) > EPSILON))
   {
     Warn(0.0, "Camera vectors are not perpendicular. "
               "Making look_at the last statement may help.");
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

void Parse_Matrix(Matrix)
MATRIX Matrix;
{
  int i, j;

  EXPECT
    CASE (LEFT_ANGLE_TOKEN)
      Matrix[0][0] = Parse_Float();
      for (i = 0; i < 4; i++)
      {
        for (j = !i ? 1 : 0; j < 3; j++)
        {
          Parse_Comma();

          Matrix[i][j] = Parse_Float();
        }

        Matrix[i][3] = (i != 3 ? 0.0 : 1.0);
      }
      GET (RIGHT_ANGLE_TOKEN);

      /* Check to see that we aren't scaling any dimension by zero */
      for (i = 0; i < 3; i++)
      {
        if (fabs(Matrix[0][i]) < EPSILON && fabs(Matrix[1][i]) < EPSILON &&
            fabs(Matrix[2][i]) < EPSILON)
        {
          Warn(0.0,"Illegal matrix column: Scale by 0.0. Changed to 1.0.");
          Matrix[i][i] = 1.0;
        }
      }
      EXIT
    END_CASE

    OTHERWISE
      Parse_Error (LEFT_ANGLE_TOKEN);
    END_CASE
  END_EXPECT
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

static
TRANSFORM *Parse_Transform ()
  {
   MATRIX Local_Matrix;
   TRANSFORM *New, Local_Trans;
   VECTOR Local_Vector;

   Parse_Begin ();
   New = Create_Transform ();

   EXPECT
     CASE(TRANSFORM_ID_TOKEN)
       Compose_Transforms (New, (TRANSFORM *)Token.Constant_Data);
       EXIT
     END_CASE

     CASE (TRANSLATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Translation_Transform(&Local_Trans, Local_Vector);
       Compose_Transforms (New, &Local_Trans);
     END_CASE

     CASE (ROTATE_TOKEN)
       Parse_Vector (Local_Vector);
       Compute_Rotation_Transform(&Local_Trans, Local_Vector);
       Compose_Transforms (New, &Local_Trans);
     END_CASE

     CASE (SCALE_TOKEN)
       Parse_Scale_Vector (Local_Vector);
       Compute_Scaling_Transform(&Local_Trans, Local_Vector);
       Compose_Transforms (New, &Local_Trans);
     END_CASE

     CASE (MATRIX_TOKEN)
       Parse_Matrix(Local_Matrix);
       Compute_Matrix_Transform(&Local_Trans, Local_Matrix);
       Compose_Transforms (New, &Local_Trans);
     END_CASE

     OTHERWISE
       UNGET
       EXIT
     END_CASE
   END_EXPECT

   Parse_End ();
   return (New);
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

void Parse_Declare ()
  {
  VECTOR Local_Vector;
  COLOUR *Local_Colour;
  PIGMENT *Local_Pigment;
  TNORMAL *Local_Tnormal;
  FINISH *Local_Finish;
  HALO *Local_Halo = NULL;
  TEXTURE *Local_Texture, *Temp_Texture;
  TRANSFORM *Local_Trans;
  OBJECT *Local_Object;
  CAMERA *Local_Camera;
  char *Temp_Data;
  int Previous;

  struct Constant_Struct *Constant_Ptr = NULL;

  EXPECT
    CASE (IDENTIFIER_TOKEN)
      if (++Number_Of_Constants >= Max_Constants)
      {
        if (Max_Constants >= INT_MAX/2)
        {
          Error("Too many constants declared.\n");
        }

        Max_Constants *= 2;

        Constants = (struct Constant_Struct *)POV_REALLOC(Constants, (Max_Constants+1) * sizeof(struct Constant_Struct), "constants table");
      }

      Constant_Ptr = &(Constants[Number_Of_Constants]);
      Constant_Ptr->Constant_Type = IDENTIFIER_TOKEN;
      EXIT
    END_CASE

    CASE4 (TNORMAL_ID_TOKEN, FINISH_ID_TOKEN, TEXTURE_ID_TOKEN, OBJECT_ID_TOKEN)
    CASE4 (COLOUR_MAP_ID_TOKEN, TRANSFORM_ID_TOKEN, CAMERA_ID_TOKEN, PIGMENT_ID_TOKEN)
    CASE4 (SLOPE_MAP_ID_TOKEN,NORMAL_MAP_ID_TOKEN,TEXTURE_MAP_ID_TOKEN,COLOUR_ID_TOKEN)
    CASE3 (PIGMENT_MAP_ID_TOKEN, HALO_ID_TOKEN,STRING_ID_TOKEN)
      Constant_Ptr = &(Constants[Token.Constant_Index]);
      EXIT
    END_CASE

    CASE2 (VECTOR_FUNCT_TOKEN, FLOAT_FUNCT_TOKEN)
      switch(Token.Function_Id)
        {
         case VECTOR_ID_TOKEN:
         case FLOAT_ID_TOKEN:
           Constant_Ptr = &(Constants[Token.Constant_Index]);
           break;

         default:
           Parse_Error(IDENTIFIER_TOKEN);
           break;
        }
      EXIT
    END_CASE

    OTHERWISE
      Parse_Error(IDENTIFIER_TOKEN);
    END_CASE
  END_EXPECT

  Previous = Token.Token_Id;

  GET (EQUALS_TOKEN);

  EXPECT
    CASE_COLOUR
      Local_Colour = Create_Colour();
      Ok_To_Declare = FALSE;
      Parse_Colour (*Local_Colour);
      Ok_To_Declare = TRUE;
      if (Test_Redefine(Previous,COLOUR_ID_TOKEN))
        Destroy_Colour((COLOUR *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = (char *) Local_Colour;
      Constant_Ptr->Constant_Type = COLOUR_ID_TOKEN;
      EXIT
    END_CASE

    CASE_VECTOR
      Ok_To_Declare = FALSE;
      Have_Vector = FALSE;
      Parse_Vector_Float (Local_Vector);
      if (Have_Vector)
        {
         if (Test_Redefine(Previous,VECTOR_FUNCT_TOKEN))
           Destroy_Vector((VECTOR *)Constant_Ptr->Constant_Data);
         Constant_Ptr->Constant_Type = VECTOR_ID_TOKEN;
         Constant_Ptr->Constant_Data = (char *) Create_Vector();
         Assign_Vector(Constant_Ptr->Constant_Data,Local_Vector);
        }
      else
        {
         if (Test_Redefine(Previous,FLOAT_FUNCT_TOKEN))
           Destroy_Float((DBL *)Constant_Ptr->Constant_Data);
         Constant_Ptr->Constant_Type = FLOAT_ID_TOKEN;
                        Constant_Ptr->Constant_Data = (char *) Create_Float();
         *((DBL *) Constant_Ptr->Constant_Data) = Local_Vector[X];
        }
      Ok_To_Declare = TRUE;
      EXIT
    END_CASE

    CASE (PIGMENT_TOKEN)
      Local_Pigment = Copy_Pigment((Default_Texture->Pigment));
      Parse_Begin ();
      Parse_Pigment (&Local_Pigment);
      Parse_End ();
      if (Test_Redefine(Previous,PIGMENT_ID_TOKEN))
        Destroy_Pigment((PIGMENT *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = PIGMENT_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *)Local_Pigment;
      EXIT
    END_CASE

    CASE (TNORMAL_TOKEN)
      Local_Tnormal = Copy_Tnormal((Default_Texture->Tnormal));
      Parse_Begin ();
      Parse_Tnormal (&Local_Tnormal);
      Parse_End ();
      if (Test_Redefine(Previous,TNORMAL_ID_TOKEN))
        Destroy_Tnormal((TNORMAL *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = TNORMAL_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *) Local_Tnormal;
      EXIT
    END_CASE

    CASE (FINISH_TOKEN)
      Local_Finish = Copy_Finish((Default_Texture->Finish));
      Parse_Finish (&Local_Finish);
      if (Test_Redefine(Previous,FINISH_ID_TOKEN))
        Destroy_Finish((FINISH *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = FINISH_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *) Local_Finish;
      EXIT
    END_CASE

    CASE (HALO_TOKEN)
      Local_Halo = Copy_Halo((Default_Texture->Halo));
      Parse_Halo (&Local_Halo);
      if (Test_Redefine(Previous,HALO_ID_TOKEN))
        Destroy_Halo((HALO *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = HALO_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *) Local_Halo;
      EXIT
    END_CASE

    CASE (CAMERA_TOKEN)
      Local_Camera = Copy_Camera(Default_Camera);
      Parse_Camera (&Local_Camera);
      if (Test_Redefine(Previous,CAMERA_ID_TOKEN))
        Destroy_Camera((CAMERA *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = CAMERA_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *) Local_Camera;
      EXIT
    END_CASE

    CASE (TEXTURE_TOKEN)
      Parse_Begin ();
      Local_Texture = Parse_Texture ();
      Parse_End ();
      Temp_Texture=NULL;
      Link_Textures(&Temp_Texture, Local_Texture);
      Ok_To_Declare = FALSE;
      EXPECT
        CASE (TEXTURE_TOKEN)
          Parse_Begin ();
          Local_Texture = Parse_Texture ();
          Parse_End ();
          Link_Textures(&Temp_Texture, Local_Texture);
        END_CASE

        OTHERWISE
          UNGET
          EXIT
        END_CASE
      END_EXPECT

      if (Test_Redefine(Previous,TEXTURE_ID_TOKEN))
      {
        Destroy_Textures((TEXTURE *)Constant_Ptr->Constant_Data);
      }
      Constant_Ptr->Constant_Type = TEXTURE_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *)Temp_Texture;
      Ok_To_Declare = TRUE;
      EXIT
    END_CASE

    CASE (COLOUR_MAP_TOKEN)
      Temp_Data=(char *) Parse_Colour_Map ();
      if (Test_Redefine(Previous,COLOUR_MAP_ID_TOKEN))
        Destroy_Blend_Map((BLEND_MAP *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = COLOUR_MAP_ID_TOKEN;
      Constant_Ptr->Constant_Data = Temp_Data;
      EXIT
    END_CASE

    CASE (PIGMENT_MAP_TOKEN)
      Temp_Data = (char *) Parse_Blend_Map (PIGMENT_TYPE,NO_PATTERN);
      if (Test_Redefine(Previous,PIGMENT_MAP_ID_TOKEN))
        Destroy_Blend_Map((BLEND_MAP *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = PIGMENT_MAP_ID_TOKEN;
      Constant_Ptr->Constant_Data = Temp_Data;
      EXIT
    END_CASE

    CASE (SLOPE_MAP_TOKEN)
      Temp_Data = (char *) Parse_Blend_Map (SLOPE_TYPE,NO_PATTERN);
      Constant_Ptr->Constant_Type = SLOPE_MAP_ID_TOKEN;
      if (Test_Redefine(Previous,SLOPE_MAP_ID_TOKEN))
        Destroy_Blend_Map((BLEND_MAP *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = Temp_Data;
      EXIT
    END_CASE

    CASE (TEXTURE_MAP_TOKEN)
      Temp_Data = (char *) Parse_Blend_Map (TEXTURE_TYPE,NO_PATTERN);
      Constant_Ptr->Constant_Type = TEXTURE_MAP_ID_TOKEN;
      if (Test_Redefine(Previous,TEXTURE_MAP_ID_TOKEN))
        Destroy_Blend_Map((BLEND_MAP *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = Temp_Data;
      EXIT
    END_CASE

    CASE (NORMAL_MAP_TOKEN)
      Temp_Data= (char *) Parse_Blend_Map (NORMAL_TYPE,NO_PATTERN);
      Constant_Ptr->Constant_Type = NORMAL_MAP_ID_TOKEN;
      if (Test_Redefine(Previous,NORMAL_MAP_ID_TOKEN))
        Destroy_Blend_Map((BLEND_MAP *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = Temp_Data;
      EXIT
    END_CASE

    CASE (RAINBOW_TOKEN)
      Temp_Data= (char *) Parse_Rainbow();
      Constant_Ptr->Constant_Type = RAINBOW_ID_TOKEN;
      if (Test_Redefine(Previous,RAINBOW_ID_TOKEN))
        Destroy_Rainbow((RAINBOW *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = Temp_Data;
      EXIT
    END_CASE

    CASE (FOG_TOKEN)
      Temp_Data= (char *) Parse_Fog();
      if (Test_Redefine(Previous,FOG_ID_TOKEN))
        Destroy_Fog((FOG *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = Temp_Data;
      Constant_Ptr->Constant_Type = FOG_ID_TOKEN;
      EXIT
    END_CASE

    CASE (ATMOSPHERE_TOKEN)
      Temp_Data= (char *) Parse_Atmosphere();
      if (Test_Redefine(Previous,ATMOSPHERE_ID_TOKEN))
        Destroy_Atmosphere((ATMOSPHERE *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = Temp_Data;
      Constant_Ptr->Constant_Type = ATMOSPHERE_ID_TOKEN;
      EXIT
    END_CASE

    CASE (SKYSPHERE_TOKEN)
      Temp_Data= (char *) Parse_Skysphere();
      if (Test_Redefine(Previous,SKYSPHERE_ID_TOKEN))
        Destroy_Skysphere((SKYSPHERE *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Data = Temp_Data;
      Constant_Ptr->Constant_Type = SKYSPHERE_ID_TOKEN;
      EXIT
    END_CASE

    CASE (TRANSFORM_TOKEN)
      Local_Trans = Parse_Transform ();
      if (Test_Redefine(Previous,TRANSFORM_ID_TOKEN))
        Destroy_Transform((TRANSFORM *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = TRANSFORM_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *) Local_Trans;
      EXIT
    END_CASE

    CASE4 (STRING_LITERAL_TOKEN,CHR_TOKEN,SUBSTR_TOKEN,STR_TOKEN)
    CASE4 (CONCAT_TOKEN,STRUPR_TOKEN,STRLWR_TOKEN,STRING_ID_TOKEN)
      UNGET
      Temp_Data= Parse_String();
      if (Test_Redefine(Previous,STRING_ID_TOKEN))
        POV_FREE(Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = STRING_ID_TOKEN;
      Constant_Ptr->Constant_Data = Temp_Data;
      EXIT
    END_CASE

    OTHERWISE
      UNGET
      Local_Object = Parse_Object ();
      if (Test_Redefine(Previous,OBJECT_ID_TOKEN))
        Destroy_Object((OBJECT *)Constant_Ptr->Constant_Data);
      Constant_Ptr->Constant_Type = OBJECT_ID_TOKEN;
      Constant_Ptr->Constant_Data = (char *) Local_Object;
      EXIT
    END_CASE

  END_EXPECT
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

static void Link (New_Object, Field, Old_Object_List)
  OBJECT *New_Object, **Field, **Old_Object_List;
  {
  *Field = *Old_Object_List;
  *Old_Object_List = New_Object;
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

void Link_Textures (Old_Textures, New_Textures)
  TEXTURE **Old_Textures;
  TEXTURE  *New_Textures;
  {
   TEXTURE *Layer;
   
   if (New_Textures == NULL)
     return;

   if ((*Old_Textures) != NULL)
   {
      if ((*Old_Textures)->Type != PLAIN_PATTERN) 
      {
         Error("Cannot layer over a patterned texture.\n");
      }
   }
   for (Layer = New_Textures ;
        Layer->Next != NULL ;
        Layer = (TEXTURE *)Layer->Next)
     {}

   Layer->Next = (TPATTERN *)*Old_Textures;
   *Old_Textures = New_Textures;

   if ((New_Textures->Type != PLAIN_PATTERN) && (New_Textures->Next != NULL))
   {
      Error("Cannot layer a patterned texture over another.\n");
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

static
char *Get_Token_String (Token_Id)
  TOKEN Token_Id;
  {
  register int i;

  for (i = 0 ; i < LAST_TOKEN ; i++)
     if (Reserved_Words[i].Token_Number == Token_Id)
        return (Reserved_Words[i].Token_Name);
  return ("");
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

static int Test_Redefine(Previous,a)
  int Previous;
  int a;
  {
  char *old, *new;

  if (Previous == IDENTIFIER_TOKEN)
    return (FALSE);
  if (Previous != a)
    {old = Get_Token_String (Previous);
     new = Get_Token_String (a);
     Error ("Attempted to redefine %s as %s.", old, new);
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
* CHANGES
*
******************************************************************************/

void Parse_Error (Token_Id)
  TOKEN Token_Id;
  {
  char *expected;

  expected = Get_Token_String (Token_Id);
  Parse_Error_Str(expected);
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

void Parse_Error_Str (str)
  char *str;
  {
   Where_Error ();
   Error_Line("%s expected but", str);
   Found_Instead ();
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

static void Found_Instead ()
{
  char *found;
  
  Stage=STAGE_FOUND_INSTEAD;

  switch(Token.Token_Id)
  {
    case IDENTIFIER_TOKEN:
      Error (" undeclared identifier '%s' found instead.\n", Token.Token_String);
      break;
      
    case VECTOR_FUNCT_TOKEN:
      found = Get_Token_String (Token.Function_Id);
      Error (" vector function '%s' found instead.\n", found);
      break;
      
    case FLOAT_FUNCT_TOKEN:
      found = Get_Token_String (Token.Function_Id);
      Error (" float function '%s' found instead.\n", found);
      break;
 
    case COLOUR_KEY_TOKEN:
      found = Get_Token_String (Token.Function_Id);
      Error (" color keyword '%s' found instead.\n", found);
      break;
 
    default:
      found = Get_Token_String (Token.Token_Id);
      Error (" %s found instead.\n", found);
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

void Warn_State (Token_Id,Type)
  TOKEN Token_Id, Type;
  {
  char *found;
  char *should;

  found = Get_Token_String (Token_Id);
  should = Get_Token_String (Type);
  Warning (1.5, "%s:%d: warning: Found %s that should be in %s statement.\n",
           Token.Filename, Token.Token_Line_No+1, found, should);
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

void Warn (Level, str)
  DBL Level;
  char *str;
  {
  if (opts.Language_Version < Level)
    return;

  Warning(Level, "%s:%d: warning: %s\n", Token.Filename, Token.Token_Line_No+1, str);
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

void MAError (str,size)
  char *str;
  size_t size;
  {
  Error ("Out of memory.  Cannot allocate %ld bytes for %s.\n",size,str);
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

/* Write a token out to the token file */

void Write_Token (Token_Id, Data_File)
  TOKEN Token_Id;
  DATA_FILE *Data_File;

  {
   Token.Token_Line_No = Data_File->Line_Number;
   Token.Filename = Data_File->Filename;
   Token.Token_String = String;
   Token.Constant_Data = NULL;
   Token.Token_Id = Token_Id;

   Token.Constant_Index = (int) Token.Token_Id - (int) LAST_TOKEN;

   if (Token.Constant_Index >= 0)
   {
      if (Token.Constant_Index <= Number_Of_Constants)
      {
        Token.Constant_Data = Constants[Token.Constant_Index].Constant_Data;
        Token.Token_Id=Constants[Token.Constant_Index].Constant_Type;
      }
      else 
      {
         Token.Token_Id = IDENTIFIER_TOKEN;
      }
   }

   Token.Function_Id = Token.Token_Id;
   if (Token.Token_Id < FLOAT_FUNCT_TOKEN)
     Token.Token_Id = FLOAT_FUNCT_TOKEN;
   else
     if (Token.Token_Id < VECTOR_FUNCT_TOKEN)
       Token.Token_Id = VECTOR_FUNCT_TOKEN;
     else
       if (Token.Token_Id < COLOUR_KEY_TOKEN)
         Token.Token_Id = COLOUR_KEY_TOKEN;
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

static void Post_Process (Object,Parent)
OBJECT *Object, *Parent;
{
  DBL Volume;
  OBJECT *Sib;

  if (Object == NULL)
    return;

  if (Parent != NULL)
  {
    if (Object->Texture == NULL)
      Object->Texture = Copy_Texture_Pointer (Parent->Texture);

/*
    else
      if (Parent->Texture != NULL)
      {
        Local_Texture = Copy_Textures (Parent->Texture);
        Link_Textures (&(Object->Texture), Local_Texture);
      }
*/ /* Removed for backward compat with 1.0.  May put back in. CEY 12/92 */

    if (Test_Flag(Parent, NO_SHADOW_FLAG))
      Set_Flag(Object, NO_SHADOW_FLAG);
  }

  if ((Object->Texture == NULL) &&
      !(Object->Type & TEXTURED_OBJECT) &&
      !(Object->Type & LIGHT_SOURCE_OBJECT))
    Object->Texture = Copy_Textures(Default_Texture);

  if (Object->Type & COMPOUND_OBJECT)
  {
    if (Object->Type & LIGHT_SOURCE_OBJECT)
    {
       ((LIGHT_SOURCE *)Object)->Next_Light_Source = Frame.Light_Sources;
       Frame.Light_Sources = (LIGHT_SOURCE *)Object;
       Frame.Number_Of_Light_Sources++;
    }

    for (Sib = ((CSG *)Object)->Children; Sib != NULL; Sib = Sib->Sibling)
      Post_Process(Sib, Object);
  }
  else
  {
    if (Object->Texture == NULL)
      Object->Texture = Copy_Textures(Default_Texture);

    if (Object->Texture->Type == PLAIN_PATTERN)
      if (Object->Texture->Tnormal != NULL)
        Object->Type |= DOUBLE_ILLUMINATE;
  }

  Post_Textures (Object->Texture);

  /* Test wether the object is finite or infinite. [DB 9/94] */

  BOUNDS_VOLUME(Volume, Object->BBox);

  if (Volume > INFINITE_VOLUME)
  {
    Set_Flag(Object, INFINITE_FLAG);
  }

  /* Test wether the object is opaque or not. [DB 8/94] */

  /*
   * Blobs and meshes have to be handled speratly because
   * of the multiple textures.
   */

  if ((Object->Methods != &Blob_Methods) &&
      (Object->Methods != &Mesh_Methods) &&
      (Test_Opacity(Object->Texture)))
  {
    Set_Flag(Object, OPAQUE_FLAG);
  }

  if (Object->Methods == &Blob_Methods)
  {
    Test_Blob_Opacity((BLOB *)Object);
  }

  if (Object->Methods == &Mesh_Methods)
  {
    Test_Mesh_Opacity((MESH *)Object);
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

static void Destroy_Constants ()
  {
   int i;
   char *Ptr;

   for (i=1; i <= Number_Of_Constants; i++)
     {
      Ptr = Constants[i].Constant_Data;
      switch (Constants[i].Constant_Type)
        {
         case COLOUR_ID_TOKEN:
           Destroy_Colour((COLOUR *)Ptr);
           break;
         case VECTOR_ID_TOKEN:
           Destroy_Vector((VECTOR *)Ptr);
           break;
         case FLOAT_ID_TOKEN:
           Destroy_Float((DBL *)Ptr);
           break;
         case PIGMENT_ID_TOKEN:
           Destroy_Pigment((PIGMENT *)Ptr);
           break;
         case TNORMAL_ID_TOKEN:
           Destroy_Tnormal((TNORMAL *)Ptr);
           break;
         case FINISH_ID_TOKEN:
           Destroy_Finish((FINISH *)Ptr);
           break;
         case HALO_ID_TOKEN:
           Destroy_Halo((HALO *)Ptr);
           break;
         case TEXTURE_ID_TOKEN:
           Destroy_Textures((TEXTURE *)Ptr);
           break;
         case OBJECT_ID_TOKEN:
           Destroy_Object((OBJECT *)Ptr);
           break;
         case COLOUR_MAP_ID_TOKEN:
         case PIGMENT_MAP_ID_TOKEN:
         case SLOPE_MAP_ID_TOKEN:
         case TEXTURE_MAP_ID_TOKEN:
         case NORMAL_MAP_ID_TOKEN:
           Destroy_Blend_Map((BLEND_MAP *)Ptr);
           break;
         case TRANSFORM_ID_TOKEN:
           Destroy_Transform((TRANSFORM *)Ptr);
           break;
         case CAMERA_ID_TOKEN:
           Destroy_Camera((CAMERA *)Ptr);
           break;
         case RAINBOW_ID_TOKEN:
           Destroy_Rainbow((RAINBOW *)Ptr);
           break;
         case FOG_ID_TOKEN:
           Destroy_Fog((FOG *)Ptr);
           break;
         case SKYSPHERE_ID_TOKEN:
           Destroy_Skysphere((SKYSPHERE *)Ptr);
           break;
         case ATMOSPHERE_ID_TOKEN:
           Destroy_Atmosphere((ATMOSPHERE *)Ptr);
           break;
         case STRING_ID_TOKEN:
           POV_FREE(Ptr);
           break;
        }
     }

     POV_FREE(Constants);
  }

/*****************************************************************************
*
* FUNCTION
*
*   Link_To_Frame
*
* INPUT
*
*   Object - Pointer to object
*   
* OUTPUT
*
*   Object
*   
* RETURNS
*   
* AUTHOR
*
*   POV-Ray Team
*   
* DESCRIPTION
*
*   -
*
* CHANGES
*
*   Sep 1994 : Added optional splitting of bounded unions if children are
*              finite. Added removing of unnecessary bounding. [DB]
*
******************************************************************************/

static void Link_To_Frame(Object)
OBJECT *Object;
{
  int finite;
  DBL Volume;
  OBJECT *This_Sib, *Next_Sib;

  if (Object == NULL)           /* patches a memory addressing error jdm mar/95 */
    return;

  /* Remove bounding object if object is cheap to intersect. [DB 8/94]  */

  if ((opts.Options & REMOVE_BOUNDS) && (Object->Bound != NULL))
  {
    if ((Object->Methods != &CSG_Union_Methods)        &&
        (Object->Methods != &CSG_Intersection_Methods) &&
        (Object->Methods != &CSG_Merge_Methods)        &&
        (Object->Methods != &Poly_Methods)             &&
        (Object->Methods != &TTF_Methods))
    {
      /* Destroy only, if bounding object is not used as clipping object. */

      if (Object->Bound != Object->Clip)
      {
        Destroy_Object(Object->Bound);
      }

      Object->Bound = NULL;

      Warn(0.0, "Unnecessary bounding object removed.");
    }
  }

  /*
   * Link the object to the frame if it's not a CSG union object,
   * if it's clipped or if bounding slabs aren't used.
   */

  if ((Object->Methods != &CSG_Union_Methods) ||
      (Object->Clip != NULL) ||
      (!opts.Use_Slabs))
  {
    Link(Object, &(Object->Sibling), &(Frame.Objects));

    return;
  }

  /*
   * [DB 8/94]
   *
   * The object is a CSG union object. It will be split if all siblings are
   * finite, i.e. the volume of the bounding box doesn't exceed a threshold.
   */

  if (Object->Bound != NULL)
  {
    /* Test if all siblings are finite. */

    finite = TRUE;

    for (This_Sib = ((CSG *)Object)->Children; This_Sib != NULL; This_Sib = This_Sib->Sibling)
    {
      BOUNDS_VOLUME(Volume, This_Sib->BBox);

      if (Volume > BOUND_HUGE)
      {
        finite = FALSE;

        break;
      }
    }

    /*
     * If the union has infinite children or splitting is not used link
     * the union to the frame.
     */

    if ((!finite) || !(opts.Options & SPLIT_UNION))
    {
      if (finite)
      {
        Warn(0.0, "CSG union unnecessarily bounded.");
      }

      Link(Object, &(Object->Sibling), &(Frame.Objects));

      return;
    }

    Warn(0.0, "Bounded CSG union split.");
  }

  /* Link all siblings of a union to the frame. */

  for (This_Sib = ((CSG *)Object)->Children; This_Sib != NULL; This_Sib = Next_Sib)
  {
    /* Link_To_Frame() changes Sibling so save it */

    Next_Sib = This_Sib->Sibling;

    /* Sibling is no longer inside a CSG object. */

    This_Sib->Type &= ~IS_CHILD_OBJECT;

    Link_To_Frame (This_Sib);
  }

/*
  Object->Texture = NULL;
*/

  Object->Sibling = NULL;

  ((CSG *)Object)->Children = NULL;

  Destroy_Object (Object);
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

void Only_In(s1,s2)
char *s1, *s2;
{
  Error("Keyword '%s' can only be used in a %s statement.",s1,s2);
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

void Not_With(s1,s2)
char *s1, *s2;
{
  Error("Keyword '%s' cannot be used %s.",s1,s2);
}

