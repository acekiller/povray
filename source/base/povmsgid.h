/*******************************************************************************
 * povmsgid.h
 *
 * This module contains all defines, typedefs, and prototypes for the POVMS.
 *
 * from Persistence of Vision Ray Tracer ('POV-Ray') version 3.7.
 * Copyright 1991-2003 Persistence of Vision Team
 * Copyright 2003-2009 Persistence of Vision Raytracer Pty. Ltd.
 * ---------------------------------------------------------------------------
 * NOTICE: This source code file is provided so that users may experiment
 * with enhancements to POV-Ray and to port the software to platforms other
 * than those supported by the POV-Ray developers. There are strict rules
 * regarding how you are permitted to use this file. These rules are contained
 * in the distribution and derivative versions licenses which should have been
 * provided with this file.
 *
 * These licences may be found online, linked from the end-user license
 * agreement that is located at http://www.povray.org/povlegal.html
 * ---------------------------------------------------------------------------
 * POV-Ray is based on the popular DKB raytracer version 2.12.
 * DKBTrace was originally written by David K. Buck.
 * DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
 * ---------------------------------------------------------------------------
 * $File: //depot/povray/smp/source/base/povmsgid.h $
 * $Revision: #58 $
 * $Change: 5387 $
 * $DateTime: 2011/01/17 15:14:56 $
 * $Author: chrisc $
 *******************************************************************************/

/*********************************************************************************
 * NOTICE
 *
 * This file is part of a BETA-TEST version of POV-Ray version 3.7. It is not
 * final code. Use of this source file is governed by both the standard POV-Ray
 * licences referred to in the copyright header block above this notice, and the
 * following additional restrictions numbered 1 through 4 below:
 *
 *   1. This source file may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd.
 *
 *   2. This notice may not be altered or removed.
 *   
 *   3. Binaries generated from this source file by individuals for their own
 *      personal use may not be re-distributed without the written permission
 *      of Persistence of Vision Raytracer Pty. Ltd. Such personal-use binaries
 *      are not required to have a timeout, and thus permission is granted in
 *      these circumstances only to disable the timeout code contained within
 *      the beta software.
 *   
 *   4. Binaries generated from this source file for use within an organizational
 *      unit (such as, but not limited to, a company or university) may not be
 *      distributed beyond the local organizational unit in which they were made,
 *      unless written permission is obtained from Persistence of Vision Raytracer
 *      Pty. Ltd. Additionally, the timeout code implemented within the beta may
 *      not be disabled or otherwise bypassed in any manner.
 *
 * The following text is not part of the above conditions and is provided for
 * informational purposes only.
 *
 * The purpose of the no-redistribution clause is to attempt to keep the
 * circulating copies of the beta source fresh. The only authorized distribution
 * point for the source code is the POV-Ray website and Perforce server, where
 * the code will be kept up to date with recent fixes. Additionally the beta
 * timeout code mentioned above has been a standard part of POV-Ray betas since
 * version 1.0, and is intended to reduce bug reports from old betas as well as
 * keep any circulating beta binaries relatively fresh.
 *
 * All said, however, the POV-Ray developers are open to any reasonable request
 * for variations to the above conditions and will consider them on a case-by-case
 * basis.
 *
 * Additionally, the developers request your co-operation in fixing bugs and
 * generally improving the program. If submitting a bug-fix, please ensure that
 * you quote the revision number of the file shown above in the copyright header
 * (see the '$Revision:' field). This ensures that it is possible to determine
 * what specific copy of the file you are working with. The developers also would
 * like to make it known that until POV-Ray 3.7 is out of beta, they would prefer
 * to emphasize the provision of bug fixes over the addition of new features.
 *
 * Persons wishing to enhance this source are requested to take the above into
 * account. It is also strongly suggested that such enhancements are started with
 * a recent copy of the source.
 *
 * The source code page (see http://www.povray.org/beta/source/) sets out the
 * conditions under which the developers are willing to accept contributions back
 * into the primary source tree. Please refer to those conditions prior to making
 * any changes to this source, if you wish to submit those changes for inclusion
 * with POV-Ray.
 *
 *********************************************************************************/


#ifndef POVMSGID_H
#define POVMSGID_H

/*
 * The following instructions are an example of how to add a POVMS option
 * to POV-Ray, e.g. a new command-line switch or whatever. In this case we
 * are discussing the best way to add the Render_Block_Size option; you
 * should be able to extrapolate from this to whatever it is you intend doing.
 *
 * Take a look at frontend/processrenderoptions.cpp, i.e. Final_Frame in the
 * RenderOptions_INI_Table table. As you see, apart from a name, you need a
 * enum for POVMS. Those are in base/povmsgid.h . Please note that the Enums
 * follow a strict convention there to allow easily tracing back a four
 * character code to the variable. The block size is part of the view/rendered,
 * so it goes into the section marked with the comment "// options handled by
 * view/renderer". As you notice, options are groups by functions and separated
 * by a blank line. Thread specs are a new option, so a new group would be
 * appropriate (just add it at the end of that section).
 * 
 * Now to the naming: The four characters are the initial characters of each
 * word in the variable. However, Sometimes that isn't enough, so the rule is
 * to use the initial characters except for the last word if there are less
 * than four characters so far, the second and third characters of the last
 * word are used as well, and those are lower case, for example:
 * 
 * kPOVAttrib_DebugConsole -> Debug CONsole -> DCon
 * 
 * However, sometimes this does not yield a valid difference, for example:
 * 
 * kPOVAttrib_PreSceneCommand -> Pre Scene COmmand -> PSCo
 * kPOVAttrib_PostSceneCommand -> Post Scene COmmand -> PSCo
 * 
 * So the rule then goes that the word that makes a difference is expanded, so
 * the "Post" and "Pre", which yields:
 * 
 * kPOVAttrib_PreSceneCommand -> PRe Scene Command -> PrSC
 * kPOVAttrib_PostSceneCommand -> POst Scene Command -> PoSC
 * 
 * However, there are exptions to this rule as well because the names on the
 * kPOVAttrib_xxx enums folow those in the INI file, which are not always
 * consistent. Hence, for example there would be:
 * 
 * kPOVAttrib_SamplingMethod -> Sampling METhod -> SMet
 * 
 * BUT, this is an anit-aliasing option, just without a prepended AnitAliasing
 * in the kPOVAttrib_xxx name (because it follows the option name). Thus, here
 * the rule is to implicitly prepend the AnitAliasing, yielding:
 * 
 * kPOVAttrib_SamplingMethod -> (Anti Aliasing) Sampling Method -> AASM
 * 
 * Then there is yet another rule for files: They should be written with an
 * implied "Name" at the end, i.e.:
 * 
 * kPOVAttrib_DebugFile -> Debug File (NAme) -> DFNa
 * 
 * The implied "Name" at the end is more for historic reasons, but as it is
 * used throughout, it should be kept as is ;-)
 * 
 * So, now that you know how to define it, make sure you select a useful name,
 * i.e. "RenderBlockSize" in the INI option, so you would get "RBSi" for the
 * four character code and an easy to read INI file.
 * 
 * After this, also decide if you want an command-line option. Look at the
 * RenderOptions_Cmd_Table table in frontend/processrenderoptions.cpp. Be
 * aware that the order has to be alphabethical in that table, and with the
 * shortest multicharacter option at the end. That is, for example the order
 * is "KFF", "KFI", "KF", "K", ***NOT*** "K", "KF", "KFF", "KFI". The reason
 * is really simple: The command-line parser picks the first match as that
 * makes it easier to implement (no look-ahead).
 * 
 * For each of the two tables, the second column defines the POVMS name and the
 * third the POVMS type. There is the additional "kUseSpecialHandler" to
 * support special handling of options - that is, parsing more complex options
 * which commonly yield more than one POVMS attribute or different attributes.
 * 
 * For the command-line argument table there is a fourth column. It provides
 * an optional binary POVMS attribute to set depending on the switch, i.e. "-"
 * sets it to "false" while a "+" and "/" set it to "true". I.e. the "UA"
 * command-line option just works via the +/- and doesn't have an argument.
 * 
 * NOTE: The default is not stored in the table at all, that is completely
 * inside the core code. That is, if the option isn't specified, it just
 * doesn't have a value.
 * 
 * The parsers will automatically handle POVMS types kPOVMSType_Float,
 * kPOVMSType_Int, kPOVMSType_CString and kPOVMSType_Bool and set POVMS
 * attributes accordingly.
 * 
 * So once you added the entries to one or both of the tables, the INI and
 * command-line parser can handle the option.
 */

/*****************************************************************************
* NOTE: If you do not understand the code below, stay away! If your compiler
* warns you about the code below, your compiler warning configuration is
* incorrect. Turn off that warning! Do not, ever, mess with the code below to
* just please your compiler. Do not, ever, complain to the POV-Team about the
* code below! Just stay away from this code, please!!!
* RATIONALE: If you do understand the code below and are concerned about
* portability, please direct your attention to the file povms.cpp. There are
* several methods there that do determine the byteorder at runtime. Please
* refer to POVMSStream_Init, POVMSStream_ReadType and POVMSStream_WriteType
* in that file for details. If you have a failure of those functions to
* report, please include as many details about your platform and compiler
* as possible!
******************************************************************************/

// POV-Ray Object Classes
enum
{
	kPOVObjectClass_Rectangle           = 'Rect',
	kPOVObjectClass_ElapsedTime         = 'ETim',

	kPOVObjectClass_IsectStat           = 'ISta',
	kPOVObjectClass_SceneCamera         = 'SCam',

	kPOVObjectClass_ShellCommand        = 'SCmd',
	kPOVObjectClass_IniOptions          = 'IniO',
	kPOVObjectClass_FrontendOptions     = 'FOpt',

	kPOVObjectClass_AnimationOptions    = 'AOpt',
	kPOVObjectClass_OutputOptions       = 'OOpt',
	kPOVObjectClass_ParserOptions       = 'POpt',
	kPOVObjectClass_RenderOptions       = 'ROpt',
	kPOVObjectClass_ParserStatistics    = 'PSta',
	kPOVObjectClass_RenderStatistics    = 'RSta',

	kPOVObjectClass_PlatformData        = 'PlaD',
	kPOVObjectClass_ControlData         = 'CtrD',
	kPOVObjectClass_ResultData          = 'ResD',
	kPOVObjectClass_PixelData           = 'PixD',
	kPOVObjectClass_FileData            = 'FilD',

	kPOVObjectClass_ParserProgress      = 'ParP',
	kPOVObjectClass_BoundingProgress    = 'BouP',
	kPOVObjectClass_PhotonProgress      = 'PhoP',
	kPOVObjectClass_RadiosityProgress   = 'RadP',
	kPOVObjectClass_RenderProgress      = 'RenP',
};

// POV-Ray Message Classes
enum
{
	kPOVMsgClass_BackendControl      = 'BCtr',
	kPOVMsgClass_SceneControl        = 'SCtr',
	kPOVMsgClass_ViewControl         = 'VCtr',
	kPOVMsgClass_SceneOutput         = 'SOut',
	kPOVMsgClass_ViewOutput          = 'VOut',
	kPOVMsgClass_ViewImage           = 'VImg',
	kPOVMsgClass_FileAccess          = 'FAcc',
};

// POV-Ray Message Identifiers
enum
{
	// BackendControl
	kPOVMsgIdent_InitInfo            = 'Info',

	kPOVMsgIdent_CreateScene         = 'CreS',
	kPOVMsgIdent_CloseScene          = 'CloS',

	// SceneControl
	kPOVMsgIdent_CreateView          = 'CreV',
	kPOVMsgIdent_CloseView           = 'CloV',

	kPOVMsgIdent_StartParser         = 'StaP',
	kPOVMsgIdent_StopParser          = 'StpP',
	kPOVMsgIdent_PauseParser         = 'PauP',
	kPOVMsgIdent_ResumeParser        = 'ResP',

	// SceneOutput
	kPOVMsgIdent_ParserStatistics    = 'PSta',

	// ViewControl
	kPOVMsgIdent_StartRender         = 'StaR',
	kPOVMsgIdent_StopRender          = 'StpR',
	kPOVMsgIdent_PauseRender         = 'PauR',
	kPOVMsgIdent_ResumeRender        = 'ResR',

	// ViewOutput
	kPOVMsgIdent_RenderStatistics    = 'RSta',

	// ViewImage
	kPOVMsgIdent_PixelSet            = 'PxSe',
	kPOVMsgIdent_PixelBlockSet       = 'PxBS',
	kPOVMsgIdent_PixelRowSet         = 'RxRS',
	kPOVMsgIdent_RectangleFrameSet   = 'ReFS',
	kPOVMsgIdent_FilledRectangleSet  = 'FiRS',

	// SceneOutput, ViewOutput
	kPOVMsgIdent_Warning             = 'Warn',
	kPOVMsgIdent_Error               = 'ErrW',
	kPOVMsgIdent_FatalError          = 'ErrF',
	kPOVMsgIdent_Debug               = 'Dbug',

	kPOVMsgIdent_Progress            = 'Prog',

	// FileAccess
	kPOVMsgIdent_FindFile            = 'FinF',
	kPOVMsgIdent_ReadFile            = 'ReaF',
	kPOVMsgIdent_CreatedFile         = 'CreF',

	// all
	kPOVMsgIdent_Done                = 'Done',
	kPOVMsgIdent_Failed              = 'Fail',

	// shell command
	kPOVMsgIdent_CmdPreParse         = 'CPrP',
	kPOVMsgIdent_CmdPostParse        = 'CPoP',
	kPOVMsgIdent_CmdPreRender        = 'CPrR',
	kPOVMsgIdent_CmdPostRender       = 'CPoR',
	kPOVMsgIdent_CmdError            = 'CErr',
	kPOVMsgIdent_CmdAbort            = 'CAbo',

	// other
	kPOVMsgIdent_ParserOptions       = 'POpt',
	kPOVMsgIdent_RenderOptions       = 'ROpt',
};

// POV-Ray Message Attributes
enum
{
	kPOVAttrib_ErrorNumber           = 'ErrN',

	kPOVAttrib_SceneId               = 'ScId',
	kPOVAttrib_ViewId                = 'ViId',

	kPOVAttrib_PlatformData          = 'PlaD',
	kPOVAttrib_MaxRenderThreads      = 'MRTh',
	kPOVAttrib_SceneCamera           = 'SCam',

	// universal use
	kPOVAttrib_EnglishText           = 'ETxt',

	// FileAccess
	kPOVAttrib_ReadFile              = 'RFil',
	kPOVAttrib_LocalFile             = 'LFil',
	kPOVAttrib_FileURL               = 'FURL',
	kPOVAttrib_CreatedFile           = 'CFil',

	// backend init
	kPOVAttrib_CoreVersion           = 'Core',
	kPOVAttrib_PlatformName          = 'Plat',
	kPOVAttrib_Official              = 'Offi',
	kPOVAttrib_PrimaryDevs           = 'Prim',
	kPOVAttrib_AssistingDevs         = 'Asst',
	kPOVAttrib_ContributingDevs      = 'Cont',
	kPOVAttrib_ImageLibVersions      = 'ILVe',

	// options handled by frontend
	kPOVAttrib_TestAbort             = 'TstA', // currently not supported by code
	kPOVAttrib_TestAbortCount        = 'TsAC', // currently not supported by code
	kPOVAttrib_PauseWhenDone         = 'PWDo',

	kPOVAttrib_ContinueTrace         = 'ConT',
	kPOVAttrib_BackupTrace           = 'BacT',

	kPOVAttrib_Verbose               = 'Verb',
	kPOVAttrib_DebugConsole          = 'DCon',
	kPOVAttrib_FatalConsole          = 'FCon',
	kPOVAttrib_RenderConsole         = 'RCon',
	kPOVAttrib_StatisticsConsole     = 'SCon',
	kPOVAttrib_WarningConsole        = 'WCon',
	kPOVAttrib_AllConsole            = 'ACon',
	kPOVAttrib_DebugFile             = 'DFNa',
	kPOVAttrib_FatalFile             = 'FFNa',
	kPOVAttrib_RenderFile            = 'RFNa',
	kPOVAttrib_StatisticsFile        = 'SFNa',
	kPOVAttrib_WarningFile           = 'WFNa',
	kPOVAttrib_AllFile               = 'AFNa',

	kPOVAttrib_Display               = 'Disp',
	kPOVAttrib_VideoMode             = 'VMod', // currently not supported by code
	kPOVAttrib_Palette               = 'Palt', // currently not supported by code
	kPOVAttrib_DisplayGamma          = 'DGam',
	kPOVAttrib_DisplayGammaType      = 'DGaT',
	kPOVAttrib_FileGamma             = 'FGam',
	kPOVAttrib_FileGammaType         = 'FGaT',
	kPOVAttrib_LegacyGammaMode       = 'LGaM',
	kPOVAttrib_WorkingGammaType      = 'WGaT',
	kPOVAttrib_WorkingGamma          = 'WGam',
	kPOVAttrib_ViewingGamma          = 'VGam',
	kPOVAttrib_DitherMethod          = 'DitM',
	kPOVAttrib_Dither                = 'Dith',

	kPOVAttrib_InitialFrame          = 'IFrm',
	kPOVAttrib_FinalFrame            = 'FFrm',
	kPOVAttrib_InitialClock          = 'IClk',
	kPOVAttrib_FinalClock            = 'FClk',
	kPOVAttrib_SubsetStartFrame      = 'SStF',
	kPOVAttrib_SubsetEndFrame        = 'SEnF',
	kPOVAttrib_CyclicAnimation       = 'CylA',
	kPOVAttrib_FieldRender           = 'FldR', // currently not supported by code
	kPOVAttrib_OddField              = 'OddF', // currently not supported by code
	kPOVAttrib_FrameStep             = 'FStp',

	kPOVAttrib_OutputToFile          = 'OToF',
	kPOVAttrib_OutputFileType        = 'OFTy',
	kPOVAttrib_OutputAlpha           = 'OAlp',
	kPOVAttrib_BitsPerColor          = 'BPCo',
	kPOVAttrib_GrayscaleOutput       = 'GOut',
	kPOVAttrib_OutputFile            = 'OFNa',
	kPOVAttrib_OutputPath            = 'OPat',
	kPOVAttrib_Compression           = 'OFCo',

	kPOVAttrib_HistogramFileType     = 'HFTy', // currently not supported by code
	kPOVAttrib_HistogramFile         = 'HFNa', // currently not supported by code
	kPOVAttrib_HistogramGridSizeX    = 'HGSX', // currently not supported by code
	kPOVAttrib_HistogramGridSizeY    = 'HGSY', // currently not supported by code

	kPOVAttrib_PreSceneCommand       = 'PrSC',
	kPOVAttrib_PreFrameCommand       = 'PrFC',
	kPOVAttrib_PostSceneCommand      = 'PoSc',
	kPOVAttrib_PostFrameCommand      = 'PoFC',
	kPOVAttrib_UserAbortCommand      = 'UAbC',
	kPOVAttrib_FatalErrorCommand     = 'FErC',
	kPOVAttrib_CommandString         = 'ComS',
	kPOVAttrib_ReturnAction          = 'RAct',

	kPOVAttrib_CreateIni             = 'CIni',
	kPOVAttrib_LibraryPath           = 'LibP',
	kPOVAttrib_IncludeIni            = 'IncI',

	// options handled by scene/parser
	kPOVAttrib_InputFile             = 'IFNa',
	kPOVAttrib_IncludeHeader         = 'IncH',

	kPOVAttrib_WarningLevel          = 'WLev',
	kPOVAttrib_Declare               = 'Decl',
	kPOVAttrib_Clock                 = 'Clck',
	kPOVAttrib_ClocklessAnimation    = 'Ckla',
	kPOVAttrib_RealTimeRaytracing    = 'RTRa',
	kPOVAttrib_Version               = 'Vers',

	// options handled by view/renderer
	kPOVAttrib_Height                = 'Heig',
	kPOVAttrib_Width                 = 'Widt',

	kPOVAttrib_Left                  = 'Left',
	kPOVAttrib_Top                   = 'Top ',
	kPOVAttrib_Right                 = 'Righ',
	kPOVAttrib_Bottom                = 'Bott',

	kPOVAttrib_Antialias             = 'Anti',
	kPOVAttrib_SamplingMethod        = 'AASM',
	kPOVAttrib_AntialiasThreshold    = 'AATh',
	kPOVAttrib_AntialiasDepth        = 'AADe',
	kPOVAttrib_Jitter                = 'AAJi',
	kPOVAttrib_JitterAmount          = 'AAJA',
	kPOVAttrib_AntialiasGamma        = 'AAGa',
	kPOVAttrib_AntialiasGammaType    = 'AAGT', // currently not supported by code
	kPOVAttrib_Quality               = 'Qual',
	kPOVAttrib_HighReproducibility   = 'HRep',

	kPOVAttrib_Bounding              = 'Boun',
	kPOVAttrib_BoundingMethod        = 'BdMe',
	kPOVAttrib_BoundingThreshold     = 'BdTh',
	kPOVAttrib_BSP_MaxDepth          = 'BspD',
	kPOVAttrib_BSP_ISectCost         = 'BspI',
	kPOVAttrib_BSP_BaseAccessCost    = 'BspB',
	kPOVAttrib_BSP_ChildAccessCost   = 'BspC',
	kPOVAttrib_BSP_MissChance        = 'BspM',
	kPOVAttrib_LightBuffer           = 'LBuf', // currently not supported by code
	kPOVAttrib_VistaBuffer           = 'VBuf', // currently not supported by code
	kPOVAttrib_RemoveBounds          = 'RmBd',
	kPOVAttrib_SplitUnions           = 'SplU',

	kPOVAttrib_CreateHistogram       = 'CHis', // currently not supported by code
	kPOVAttrib_DrawVistas            = 'DrVi', // currently not supported by code

	kPOVAttrib_PreviewStartSize      = 'PStS',
	kPOVAttrib_PreviewEndSize        = 'PEnS',

	kPOVAttrib_RadiosityFileName     = 'RaFN',
	kPOVAttrib_RadiosityFromFile     = 'RaFF',
	kPOVAttrib_RadiosityToFile       = 'RaTF',
	kPOVAttrib_RadiosityVainPretrace = 'RaVP',

	kPOVAttrib_RenderBlockSize       = 'RBSi',

	kPOVAttrib_MaxImageBufferMem     = 'MIBM', // [JG] for file backed image 

	kPOVAttrib_CameraIndex           = 'CIdx',

	// time statistics generated by frontend
	kPOVAttrib_TotalTime             = 'TotT',
	kPOVAttrib_FrameTime             = 'FTim',
	kPOVAttrib_AnimationTime         = 'ATim',

	// time statistics generated by backend
	kPOVAttrib_ParseTime             = 'ParT',
	kPOVAttrib_BoundingTime          = 'BouT',
	kPOVAttrib_PhotonTime            = 'PhoT',
	kPOVAttrib_RadiosityTime         = 'RadT',
	kPOVAttrib_TraceTime             = 'TraT',

	// statistics generated by frontend
	kPOVAttrib_CurrentFrame          = 'CurF',
	kPOVAttrib_FrameCount            = 'FCnt',
	kPOVAttrib_AbsoluteCurFrame      = 'AbsF',
	kPOVAttrib_FirstClock            = 'FirC',
	kPOVAttrib_CurrentClock          = 'CurC',
	kPOVAttrib_LastClock             = 'LasC',

	// statistics generated by scene/parser
	kPOVAttrib_FiniteObjects         = 'FiOb',
	kPOVAttrib_InfiniteObjects       = 'InOb',
	kPOVAttrib_LightSources          = 'LiSo',
	kPOVAttrib_Cameras               = 'Cama',

	// statistics generated by scene/bounding
	kPOVAttrib_BSPNodes              = 'BNod',
	kPOVAttrib_BSPSplitNodes         = 'BSNo',
	kPOVAttrib_BSPObjectNodes        = 'BONo',
	kPOVAttrib_BSPEmptyNodes         = 'BENo',
	kPOVAttrib_BSPMaxObjects         = 'BMOb',
	kPOVAttrib_BSPAverageObjects     = 'BAOb',
	kPOVAttrib_BSPMaxDepth           = 'BMDe',
	kPOVAttrib_BSPAverageDepth       = 'BADe',
	kPOVAttrib_BSPAborts             = 'BAbo',
	kPOVAttrib_BSPAverageAborts      = 'BAAb',
	kPOVAttrib_BSPAverageAbortObjects = 'BAAO',

	// statistics generated by view/render (radiosity)
	kPOVAttrib_RadGatherCount        = 'RGCt',
	kPOVAttrib_RadUnsavedCount       = 'RUCo',
	kPOVAttrib_RadReuseCount         = 'RRCt',
	kPOVAttrib_RadRayCount           = 'RYCt',
	kPOVAttrib_RadTopLevelGatherCount= 'RGCT',
	kPOVAttrib_RadTopLevelReuseCount = 'RRCT',
	kPOVAttrib_RadTopLevelRayCount   = 'RYCT',
	kPOVAttrib_RadFinalGatherCount   = 'RGCF',
	kPOVAttrib_RadFinalReuseCount    = 'RRCF',
	kPOVAttrib_RadFinalRayCount      = 'RYCF',
	kPOVAttrib_RadOctreeNodes        = 'ROcN',
	kPOVAttrib_RadOctreeLookups      = 'ROcL',
	kPOVAttrib_RadOctreeAccepts0     = 'ROc0',
	kPOVAttrib_RadOctreeAccepts1     = 'ROc1',
	kPOVAttrib_RadOctreeAccepts2     = 'ROc2',
	kPOVAttrib_RadOctreeAccepts3     = 'ROc3',
	kPOVAttrib_RadOctreeAccepts4     = 'ROc4',
	kPOVAttrib_RadOctreeAccepts5     = 'ROc5',
	// [CLi] per-pass per-recursion sample count statistics
	// (Note: Do not change the IDs of any of these "just for fun"; at several places they are computed from the first one)
	kPOVAttrib_RadSamplesP1R0        = 'RS10',
	kPOVAttrib_RadSamplesP1R1        = 'RS11',
	kPOVAttrib_RadSamplesP1R2        = 'RS12',
	kPOVAttrib_RadSamplesP1R3        = 'RS13',
	kPOVAttrib_RadSamplesP1R4ff      = 'RS14',
	kPOVAttrib_RadSamplesP2R0        = 'RS20',
	kPOVAttrib_RadSamplesP2R1        = 'RS21',
	kPOVAttrib_RadSamplesP2R2        = 'RS22',
	kPOVAttrib_RadSamplesP2R3        = 'RS23',
	kPOVAttrib_RadSamplesP2R4ff      = 'RS24',
	kPOVAttrib_RadSamplesP3R0        = 'RS30',
	kPOVAttrib_RadSamplesP3R1        = 'RS31',
	kPOVAttrib_RadSamplesP3R2        = 'RS32',
	kPOVAttrib_RadSamplesP3R3        = 'RS33',
	kPOVAttrib_RadSamplesP3R4ff      = 'RS34',
	kPOVAttrib_RadSamplesP4R0        = 'RS40',
	kPOVAttrib_RadSamplesP4R1        = 'RS41',
	kPOVAttrib_RadSamplesP4R2        = 'RS42',
	kPOVAttrib_RadSamplesP4R3        = 'RS43',
	kPOVAttrib_RadSamplesP4R4ff      = 'RS44',
	kPOVAttrib_RadSamplesP5ffR0      = 'RS50',
	kPOVAttrib_RadSamplesP5ffR1      = 'RS51',
	kPOVAttrib_RadSamplesP5ffR2      = 'RS52',
	kPOVAttrib_RadSamplesP5ffR3      = 'RS53',
	kPOVAttrib_RadSamplesP5ffR4ff    = 'RS54',
	kPOVAttrib_RadSamplesFR0         = 'RSF0',
	kPOVAttrib_RadSamplesFR1         = 'RSF1',
	kPOVAttrib_RadSamplesFR2         = 'RSF2',
	kPOVAttrib_RadSamplesFR3         = 'RSF3',
	kPOVAttrib_RadSamplesFR4ff       = 'RSF4',
	kPOVAttrib_RadWeightR0           = 'RWt0',
	kPOVAttrib_RadWeightR1           = 'RWt1',
	kPOVAttrib_RadWeightR2           = 'RWt2',
	kPOVAttrib_RadWeightR3           = 'RWt3',
	kPOVAttrib_RadWeightR4ff         = 'RWt4',
	kPOVAttrib_RadQueryCountR0       = 'RQC0',
	kPOVAttrib_RadQueryCountR1       = 'RQC1',
	kPOVAttrib_RadQueryCountR2       = 'RQC2',
	kPOVAttrib_RadQueryCountR3       = 'RQC3',
	kPOVAttrib_RadQueryCountR4ff     = 'RQC4',

	// statistics generated by view/render (photons)
	kPOVAttrib_TotalPhotonCount      = 'TPCn',
	kPOVAttrib_ObjectPhotonCount     = 'OPCn',
	kPOVAttrib_MediaPhotonCount      = 'MPCn',
	kPOVAttrib_PhotonXSamples        = 'PXSa',
	kPOVAttrib_PhotonYSamples        = 'PYSa',
	kPOVAttrib_PhotonsShot           = 'PSho',
	kPOVAttrib_PhotonsStored         = 'PSto',
	kPOVAttrib_GlobalPhotonsStored   = 'GPSt',
	kPOVAttrib_MediaPhotonsStored    = 'MPSt',
	kPOVAttrib_PhotonsPriQInsert     = 'PPQI',
	kPOVAttrib_PhotonsPriQRemove     = 'PPQR',
	kPOVAttrib_GatherPerformedCnt    = 'GPCn',
	kPOVAttrib_GatherExpandedCnt     = 'GECn',

	// render progress and statistics generated by view/render (trace)
	kPOVAttrib_Pixels                = 'Pixe',
	kPOVAttrib_PixelSamples          = 'PixS',
	kPOVAttrib_SuperSampleCount      = 'SSCn',

	// statistics generated by view/render (all)
	kPOVAttrib_Rays                  = 'Rays',
	kPOVAttrib_RaysSaved             = 'RSav',

	kPOVAttrib_TraceLevel            = 'TLev',
	kPOVAttrib_MaxTraceLevel         = 'MaxL',

	kPOVAttrib_ShadowTest            = 'ShdT',
	kPOVAttrib_ShadowTestSuc         = 'ShdS',
	kPOVAttrib_ShadowCacheHits       = 'ShdC',

	kPOVAttrib_PolynomTest           = 'PnmT',
	kPOVAttrib_RootsEliminated       = 'REli',

	kPOVAttrib_CallsToNoise          = 'CTNo',
	kPOVAttrib_CallsToDNoise         = 'CTDN',

	kPOVAttrib_MediaSamples          = 'MeSa',
	kPOVAttrib_MediaIntervals        = 'MeIn',

	kPOVAttrib_ReflectedRays         = 'RflR',
	kPOVAttrib_InnerReflectedRays    = 'IReR',
	kPOVAttrib_RefractedRays         = 'RfrT',
	kPOVAttrib_TransmittedRays       = 'TraR',

	kPOVAttrib_IsoFindRoot           = 'IFRo',
	kPOVAttrib_FunctionVMCalls       = 'FVMC',
	kPOVAttrib_FunctionVMInstrEst    = 'FVMI',

	kPOVAttrib_CrackleCacheTest      = 'CrCT',
	kPOVAttrib_CrackleCacheTestSuc   = 'CrCS',

	kPOVAttrib_ObjectIStats          = 'OISt',
	kPOVAttrib_ISectsTests           = 'ITst',
	kPOVAttrib_ISectsSucceeded       = 'ISuc',

	kPOVAttrib_MinAlloc              = 'MinA',
	kPOVAttrib_MaxAlloc              = 'MaxA',
	kPOVAttrib_CallsToAlloc          = 'CTAl',
	kPOVAttrib_CallsToFree           = 'CTFr',
	kPOVAttrib_PeakMemoryUsage       = 'PMUs',

	// subject to elimination
	kPOVAttrib_BoundingQueues        = 'BQue',
	kPOVAttrib_BoundingQueueResets   = 'BQRs',
	kPOVAttrib_BoundingQueueResizes  = 'BQRz',
	kPOVAttrib_IStackOverflow        = 'IStO',
	kPOVAttrib_ObjectName            = 'ONam',
	kPOVAttrib_ObjectID              = 'OIde',

	// time statistics and progress reporting
	kPOVAttrib_RealTime              = 'ReaT',
	kPOVAttrib_CPUTime               = 'CPUT',
	kPOVAttrib_TimeSamples           = 'TSam',

	// parser progress
	kPOVAttrib_CurrentTokenCount     = 'CTCo',

	// bounding progress
	kPOVAttrib_CurrentNodeCount      = 'CNCo',

	/// photon progress
	kPOVAttrib_CurrentPhotonCount    = 'CPCo',

	// render progress
	kPOVAttrib_PixelsPending         = 'PPen',
	kPOVAttrib_PixelsCompleted       = 'PCom',

	// render pixel data/control
	kPOVAttrib_PixelId               = 'PiId',
	kPOVAttrib_PixelSize             = 'PiSi',
	kPOVAttrib_PixelBlock            = 'PBlo',
	kPOVAttrib_PixelColors           = 'PCol',
	kPOVAttrib_PixelPositions        = 'PPos',
	kPOVAttrib_PixelSkipList         = 'PSLi',

	// scene/view error reporting and TBD
	kPOVAttrib_CurrentLine           = 'CurL',
	kPOVAttrib_LineCount             = 'LCnt',
	kPOVAttrib_AbsoluteCurrentLine   = 'AbsL',
	kPOVAttrib_FileName              = 'File',
	kPOVAttrib_State                 = 'Stat',
	kPOVAttrib_Warning               = 'Warn',
	kPOVAttrib_Line                  = 'Line',
	kPOVAttrib_Column                = 'Colu',
	kPOVAttrib_FilePosition          = 'FPos',
	kPOVAttrib_TokenName             = 'TokN',
	kPOVAttrib_Error                 = 'Erro',
	kPOVAttrib_INIFile               = 'IFil',
	kPOVAttrib_RenderOptions         = 'ROpt',
	kPOVAttrib_Identifier            = 'Iden',
	kPOVAttrib_Value                 = 'Valu',
	kPOVAttrib_ProgressStatus        = 'ProS',
	kPOVAttrib_MosaicPreviewSize     = 'MPSi',

	// Rendering order
	kPOVAttrib_RenderBlockStep       = 'RBSt',
	kPOVAttrib_RenderPattern         = 'RPat',

	// helpers
	kPOVAttrib_StartColumn           = kPOVAttrib_Left,
	kPOVAttrib_EndColumn             = kPOVAttrib_Right,
	kPOVAttrib_StartRow              = kPOVAttrib_Top,
	kPOVAttrib_EndRow                = kPOVAttrib_Bottom
};

// Add new stats ONLY at the end!!!
enum
{
	kPOVList_Stat_BicubicTest = 1,
	kPOVList_Stat_BlobTest,
	kPOVList_Stat_BlobCpTest,
	kPOVList_Stat_BlobBdTest,
	kPOVList_Stat_BoxTest,
	kPOVList_Stat_ConeCylTest,
	kPOVList_Stat_CSGIntersectTest,
	kPOVList_Stat_CSGMergeTest,
	kPOVList_Stat_CSGUnionTest,
	kPOVList_Stat_DiscTest,
	kPOVList_Stat_FractalTest,
	kPOVList_Stat_HFTest,
	kPOVList_Stat_HFBoxTest,
	kPOVList_Stat_HFTriangleTest,
	kPOVList_Stat_HFBlockTest,
	kPOVList_Stat_HFCellTest,
	kPOVList_Stat_IsosurfaceTest,
	kPOVList_Stat_IsosurfaceBdTest,
	kPOVList_Stat_IsosurfaceCacheTest,
	kPOVList_Stat_LatheTest,
	kPOVList_Stat_LatheBdTest,
	kPOVList_Stat_MeshTest,
	kPOVList_Stat_PlaneTest,
	kPOVList_Stat_PolygonTest,
	kPOVList_Stat_PrismTest,
	kPOVList_Stat_PrismBdTest,
	kPOVList_Stat_ParametricTest,
	kPOVList_Stat_ParametricBoxTest,
	kPOVList_Stat_QuardicTest,
	kPOVList_Stat_QuadPolyTest,
	kPOVList_Stat_SphereTest,
	kPOVList_Stat_SphereSweepTest,
	kPOVList_Stat_SuperellipsTest,
	kPOVList_Stat_SORTest,
	kPOVList_Stat_SORBdTest,
	kPOVList_Stat_TorusTest,
	kPOVList_Stat_TorusBdTest,
	kPOVList_Stat_TriangleTest,
	kPOVList_Stat_TTFontTest,
	kPOVList_Stat_BoundObjectTest,
	kPOVList_Stat_ClipObjectTest,
	kPOVList_Stat_BoundingBoxTest,
	kPOVList_Stat_LightBufferTest,
	kPOVList_Stat_VistaBufferTest,
	kPOVList_Stat_RBezierTest,
	kPOVList_Stat_OvusTest,
	kPOVList_Stat_Last
};

// Add new progress messages ONLY at the end!!!
enum
{
	kPOVList_Prog_CreatingBoundingSlabs = 1,
	kPOVList_Prog_CreatingVistaBuffer,
	kPOVList_Prog_CreatingLightBuffers,
	kPOVList_Prog_BuildingPhotonMaps,
	kPOVList_Prog_LoadingPhotonMaps,
	kPOVList_Prog_SavingPhotonMaps,
	kPOVList_Prog_SortingPhotons,
	kPOVList_Prog_ReclaimingMemory,
	kPOVList_Prog_WritingINIFile,
	kPOVList_Prog_WritingHistogramFile,
	kPOVList_Prog_PerformingShelloutCommand,
	kPOVList_Prog_ResumingInterruptedTrace,
	kPOVList_Prog_ProcessingFrame,
	kPOVList_Prog_Parsing,
	kPOVList_Prog_Displaying,
	kPOVList_Prog_Rendering,
	kPOVList_Prog_DoneTracing,
	kPOVList_Prog_AbortingRender,
	kPOVList_Prog_UserAbort
};

// TODO FIXME - make this enum obsolete
enum
{
	PROGRESS_CREATING_BOUNDING_SLABS = kPOVList_Prog_CreatingBoundingSlabs,
	PROGRESS_CREATING_VISTA_BUFFER = kPOVList_Prog_CreatingVistaBuffer,
	PROGRESS_CREATE_LIGHT_BUFFERS = kPOVList_Prog_CreatingLightBuffers,
	PROGRESS_BUILDING_PHOTON_MAPS = kPOVList_Prog_BuildingPhotonMaps,
	PROGRESS_LOADING_PHOTON_MAPS = kPOVList_Prog_LoadingPhotonMaps,
	PROGRESS_SAVING_PHOTON_MAPS = kPOVList_Prog_SavingPhotonMaps,
	PROGRESS_SORTING_PHOTONS = kPOVList_Prog_SortingPhotons,
	PROGRESS_RECLAIMING_MEMORY = kPOVList_Prog_ReclaimingMemory,
	PROGRESS_WRITE_INI_FILE = kPOVList_Prog_WritingINIFile,
	PROGRESS_WRITE_HISTOGRAM_FILE = kPOVList_Prog_WritingHistogramFile,
	PROGRESS_PERFORMING_SHELLOUT_COMMAND = kPOVList_Prog_PerformingShelloutCommand,
	PROGRESS_RESUMING_INTERRUPTED_TRACE = kPOVList_Prog_ResumingInterruptedTrace,
	PROGRESS_PROCESSING_FRAME = kPOVList_Prog_ProcessingFrame,
	PROGRESS_PARSING = kPOVList_Prog_Parsing,
	PROGRESS_DISPLAYING = kPOVList_Prog_Displaying,
	PROGRESS_RENDERING = kPOVList_Prog_Rendering,
	PROGRESS_DONE_TRACING = kPOVList_Prog_DoneTracing,
	PROGRESS_ABORTING_RENDER = kPOVList_Prog_AbortingRender,
	PROGRESS_USER_ABORT = kPOVList_Prog_UserAbort
};

enum
{
	kPOVList_FileType_Unknown,
	kPOVList_FileType_Targa,
	kPOVList_FileType_CompressedTarga,
	kPOVList_FileType_PNG,
	kPOVList_FileType_JPEG,
	kPOVList_FileType_PPM,
	kPOVList_FileType_BMP,
	kPOVList_FileType_OpenEXR,
	kPOVList_FileType_RadianceHDR,
	kPOVList_FileType_System,
	kPOVList_FileType_CSV, // used for histogram file
};

enum
{
	/**
	 *  No gamma handling.
	 *  This model is based on the (wrong) presumption that image file pixel values are proportional to
	 *  physical light intensities.
	 *  This is the default for POV-Ray 3.6 and earlier.
	 */
	kPOVList_GammaMode_None,
	/**
	 *  Explicit assumed_gamma-based gamma handling model, 3.6 variant.
	 *  This model is based on the (wrong) presumption that render engine maths works equally well with
	 *  both linear and gamma-encoded light intensity values.
	 *  Using assumed_gamma=1.0 gives physically realistic results.
	 *  Input image files without implicit or explicit gamma information will be presumed to match assumed_gamma,
	 *  i.e. they will not be gamma corrected.
	 *  This is the mode used by POV-Ray 3.6 if assumed_gamma is specified.
	 */
	kPOVList_GammaMode_AssumedGamma36,
	/**
	 *  Explicit assumed_gamma-based gamma handling model, 3.7 variant.
	 *  This model is based on the (wrong) presumption that render engine maths works equally well with
	 *  both linear and gamma-encoded light intensity values.
	 *  Using assumed_gamma=1.0 gives physically realistic results.
	 *  Input image files without implicit or explicit gamma information will be presumed to match official
	 *  recommendations for the respective file format; files for which no official recommendations exists
	 *  will be presumed to match assumed_gamma.
	 *  This is the mode used by POV-Ray 3.7 if assumed_gamma is specified.
	 */
	kPOVList_GammaMode_AssumedGamma37,
	/**
	 *  Implicit assumed_gamma-based gamma handling model, 3.7 variant.
	 *  This model is functionally idential to kPOVList_GammaMode_AssumedGamma37 except that it also serves as a marker
	 *  that assumed_gamma has not been set explicitly.
	 *  This is the default for POV-Ray 3.7 and later.
	 */
	kPOVList_GammaMode_AssumedGamma37Implied,
};

enum
{
	kPOVList_GammaType_Unknown,
	kPOVList_GammaType_Neutral,
	kPOVList_GammaType_PowerLaw,
	kPOVList_GammaType_SRGB
};

enum
{
	kPOVList_DitherMethod_None,
	kPOVList_DitherMethod_Diffusion1D,
	kPOVList_DitherMethod_Diffusion2D,
	kPOVList_DitherMethod_FloydSteinberg,
	kPOVList_DitherMethod_Bayer2x2,
	kPOVList_DitherMethod_Bayer3x3,
	kPOVList_DitherMethod_Bayer4x4,
};

#endif // POVMSGID_H
