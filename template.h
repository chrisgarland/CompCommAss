/*******************************************************************************

    Sample header file for template.h 

*******************************************************************************/
#include <string.h>
#include <cnet.h>

/************************************************************
 * Constant data structure. 
 *
 * NOTE: These are just examples 
 *       They are currently not used.
 * 
 ************************************************************/
/**/
/*
 *#define dt_data      (1)
 *#define dt_hello     (2)
 *#define dt_hello_ack (3)
 *
 *#define R_TABLE_SIZE    30
 *#define MAX_FRAME_SIZE  1024
 */

#define NUM_NODES       7
#define MAX_MSG_SIZE    256

/*******************************************************************************
 * Data structure. 
 *
 * NOTE: the following are just example/suggestion. There 
 *       might be missing or extra data field member. 
 *       They are currently not used.
 * 
 ******************************************************************************/
typedef enum {
        DL_DATA,
        DL_ACK
} Framekind_T;

typedef struct {
        char data[MAX_MSG_SIZE];
} Msg_T;

typedef struct {
        CnetAddr    src_addr;
        CnetAddr    dest_addr;
        size_t      msg_length;       	
        Msg_T       msg;
} Packet_T;

typedef struct {
        Framekind_T kind;
        int         checksum;  	
        int         seq;
        Packet_T    payload;
} Frame_T;

/**/

/************************************************************
 * Variable. 
 *  Note: global variables are allow for simulation. They are
 *  useful if you want to aviod using pointers parameters for 
 *  functions.
 ************************************************************/
/*
 *Segment current_seg;
 *Packet current_pkt;
 *Frame current_frame;
 */

/************************************************************
 * Function Prototypes. 
 *
 ************************************************************/
void reboot_host();
void reboot_router();
static void prompt(int);
static void transmit_frame(int link, Frame_T f, size_t length);

/* Data Link Layer */
void network_downto_datalink(int link, Packet_T packet, size_t length);
static void frame_arrived( CnetEvent, CnetTimerID, CnetData );

/* Network Layer */
void transport_downto_network( CnetAddr, Msg_T, size_t);
void datalink_upto_network(int link, Packet_T packet, size_t length );

/* Transport Layer */
void application_downto_transport( CnetAddr, Msg_T, size_t );
void network_upto_transport( int*, char*, size_t );

/* Application Layer */
//static void keyboard(CnetEvent, CnetTimerID, CnetData );
void transport_upto_application(int*, char*, size_t );
static void application_ready(CnetEvent ev, CnetTimerID timer, CnetData data);

