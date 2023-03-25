/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  FR_PANEL                         1       /* callback function: OnFrPanel */
#define  FR_PANEL_SWITCHPANELS            2       /* control type: binary, callback function: OnSwitchPanels */
#define  FR_PANEL_KRS_HPF_GRAPH           3       /* control type: graph, callback function: (none) */
#define  FR_PANEL_BUTTERWORTH_GRAPH       4       /* control type: graph, callback function: (none) */
#define  FR_PANEL_WINDOWED_SIGNAL         5       /* control type: graph, callback function: (none) */
#define  FR_PANEL_Original_Signal_Graph   6       /* control type: graph, callback function: (none) */
#define  FR_PANEL_LoadFRPANEL             7       /* control type: command, callback function: OnLoadFRPANEL */
#define  FR_PANEL_TIPFEREASTRA            8       /* control type: ring, callback function: (none) */
#define  FR_PANEL_NOfPoints               9       /* control type: slide, callback function: (none) */
#define  FR_PANEL_SPECTRUM_NEFILTRAT      10      /* control type: graph, callback function: (none) */
#define  FR_PANEL_SPECTRUM_BUTTERWORTH    11      /* control type: graph, callback function: (none) */
#define  FR_PANEL_SPECTRUM_KRS_GRAPH      12      /* control type: graph, callback function: (none) */
#define  FR_PANEL_IDC_FREQ_PEAK_3         13      /* control type: numeric, callback function: (none) */
#define  FR_PANEL_IDC_FREQ_PEAK_2         14      /* control type: numeric, callback function: (none) */
#define  FR_PANEL_IDC_FREQ_PEAK           15      /* control type: numeric, callback function: (none) */
#define  FR_PANEL_IDC_POWER_PEAK_3        16      /* control type: numeric, callback function: (none) */
#define  FR_PANEL_IDC_POWER_PEAK_2        17      /* control type: numeric, callback function: (none) */
#define  FR_PANEL_IDC_POWER_PEAK          18      /* control type: numeric, callback function: (none) */
#define  FR_PANEL_APLICA_CB               19      /* control type: command, callback function: Fereastra */
#define  FR_PANEL_FRECV_LOWERCUTOFF       20      /* control type: numeric, callback function: OnLowerCutOffCB */
#define  FR_PANEL_FRECV_UPPERCUTOFF       21      /* control type: numeric, callback function: OnUpperCutOffCB */
#define  FR_PANEL_TIMER                   22      /* control type: timer, callback function: OnTimer */

#define  PANEL                            2       /* callback function: OnPanel */
#define  PANEL_GRAPH_FILTERED_DATA        2       /* control type: graph, callback function: (none) */
#define  PANEL_GRAPH_RAW_DATA             3       /* control type: graph, callback function: (none) */
#define  PANEL_IDC_ZEROS                  4       /* control type: numeric, callback function: (none) */
#define  PANEL_IDC_MEDIANA                5       /* control type: numeric, callback function: (none) */
#define  PANEL_IDC_MEAN                   6       /* control type: numeric, callback function: (none) */
#define  PANEL_IDC_MAX                    7       /* control type: numeric, callback function: (none) */
#define  PANEL_IDC_DISPERSIE              8       /* control type: numeric, callback function: (none) */
#define  PANEL_IDC_MAX_INDEX              9       /* control type: numeric, callback function: (none) */
#define  PANEL_IDC_MIN_INDEX              10      /* control type: numeric, callback function: (none) */
#define  PANEL_IDC_MIN                    11      /* control type: numeric, callback function: (none) */
#define  PANEL_LoadButton                 12      /* control type: command, callback function: OnLoadButtonCB */
#define  PANEL_HIST_GRAPH                 13      /* control type: graph, callback function: (none) */
#define  PANEL_NEXT_CB                    14      /* control type: command, callback function: ON_NEXT */
#define  PANEL_PREV_CB                    15      /* control type: command, callback function: ON_PREV */
#define  PANEL_STOP                       16      /* control type: numeric, callback function: (none) */
#define  PANEL_START                      17      /* control type: numeric, callback function: (none) */
#define  PANEL_FILTRU                     18      /* control type: ring, callback function: (none) */
#define  PANEL_APLICA_CB                  19      /* control type: command, callback function: Filtru */
#define  PANEL_ALPHA_SET                  20      /* control type: numeric, callback function: (none) */
#define  PANEL_SWITCHPANELS               21      /* control type: binary, callback function: OnSwitchPanels */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK Fereastra(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK Filtru(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ON_NEXT(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK ON_PREV(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFrPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLoadButtonCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLoadFRPANEL(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLowerCutOffCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSwitchPanels(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnUpperCutOffCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif