#include "./template.h"
#include <stdlib.h>

#define FRAME_HEADER_SIZE  ((sizeof(Frame_T)) - (sizeof(Msg_T)))
#define FRAME_SIZE(f)      ((FRAME_HEADER_SIZE) + (f.payload.msg_length))
/*******************************************************************************
    
    Sample template for CC200 assignment

    This is a template cnet source (for CC200 assignment) which demonstrate the 
    TCP/IP layered design approach. This program is modified from the keyboard 
    cnet protocol source file example given by CNET. The program is a simple 
    cnet protocol source file which demonstrates the use of the keyboard event 
    and the Physical Layer in cnet.

    To run this demo, run  cnet TEMPLATE, click on both node icons to open
    their output windows, click the 'Run' button, click in one of the node's
    output windows, type your name and finally press RETURN.

    Note that this simple ``protocol'' will not work if there is corruption
    on the link (probframecorrupt != 0). Can you see why?

    Note that this simple 'protocol' only send message to a fix destination, so 
    you need to (MUST) modify the code to enable different destination node.

*******************************************************************************/
static int              seqno           = 0;

int routing_table[NUM_NODES][NUM_NODES] = {
        {0, 1, 2, 3, 4, 1, 1},
        {1, 0, 1, 1, 1, 2, 3},
        {1, 1, 0, 1, 1, 1, 1},
        {1, 1, 1, 0, 2, 1, 1},
        {1, 1, 1, 2, 0, 1, 1}, 
        {1, 1, 1, 1, 1, 0, 1},
        {1, 1, 1, 1, 1, 1, 0}
};

static void prompt(int inc)
{
    static int n=0;

    n += inc;
    printf("%s.%d> ", nodeinfo.nodename, n);
}

/***************************************************************************
 * Application Layer 
 *
 * Note: Currently using keyboard for development & testing only, 
 * but later will have to use CNET enable application to generate msg for 
 * simulation 
 *
 ****************************************************************************/

static void keyboard(CnetEvent ev, CnetTimerID timer, CnetData data)
{
        CnetAddr        src;
        CnetAddr        dest            = nodeinfo.nodenumber;
        Msg_T           msg;
        size_t          length;

        src = nodeinfo.nodenumber;
        while(dest == nodeinfo.nodenumber)
                dest = CNET_rand() % NUM_NODES;

        length	= sizeof(msg.data);
        CHECK(CNET_read_keyboard(msg.data, &length));

        if(length > 1) {			/* not just a blank msg? */
                printf("\tsending %d bytes - \"%s\" to node: %d\n", length, 
                                msg.data, dest);
                /*CHECK(CNET_write_physical(1, msg, &length));*/
                application_downto_transport(dest, msg, length);
                prompt(1);
        }
        else {
                prompt(0);
        }
}

void transport_upto_application(int *link, char* msg, size_t length )
{
   /*
    * NOTE: received msg from transport layer...  
    *
    */
    printf("\treceived %d bytes on link %d - \"%s\"\n",length,*link,msg);
    prompt(0);
}

/*
static void application_ready(CnetEvent ev, CnetTimerID timer, CnetData data)
{
    char msg[MAX_MESSAGE_SIZE];
    int length;
    CnetAddr destaddr;

    length  = sizeof(msg);
    CHECK(CNET_read_application(&destaddr, (char *)&msg, &length));
    CHECK(CNET_disable_application(ALLNODES));

    application_downto_transport(1, msg, &length);
}
*/






/**************************************************************************** 
 * Transport layer 
 * 
 * If your assignment is to develop transport layer protocol, then the codes
 * are inserted in the following functions. You may create other 
 * supporting/utilite function if required.
 * 
 * Note: this layer is to create a Segment, 
 *       Segment <- (transport layer header) + (msg from the application layer)
 *
 *****************************************************************************/

void application_downto_transport(CnetAddr dest, Msg_T msg, size_t length)
{
   /*
    * Construct segment
    * NOTE: encapsulate application msg into Segment here... 
    *
    */


    /*
     * Note: Should hand Segment and destination to the network layer
     */
    transport_downto_network(dest, msg, length);
}

void network_upto_transport(int *link, char* segment, size_t length )
{
    /*
     * NOTE: retrieve msg from Segment here & pass it backup to
     * the application layer.  
     *
     */
    transport_upto_application(link, segment, length );
}






/*****************************************************************************
 * Network layer
 * 
 * If your assignment is to develop routing protocol, then the codes
 * are inserted in the following functions. You may create other 
 * supporting/utilite function if required.
 * 
 * Note: this layer is to create a Packet/Datagram, 
 *       Packet <- (network layer header) + (Segment)
 *
 *****************************************************************************/

void transport_downto_network(CnetAddr dest, Msg_T msg, size_t length)
{
        Packet_T packet;
        int      link;

        memcpy(&packet.msg, &msg, sizeof(msg));
        
        packet.src_addr   = nodeinfo.nodenumber;
        packet.dest_addr  = dest;
        packet.msg_length = length;
        link              = routing_table[nodeinfo.nodenumber][dest];

        network_downto_datalink(link, packet, length);
}


void datalink_upto_network(int link, Packet_T packet, size_t length )
{
        int     src     = (int) packet.src_addr;
        int     dest    = (int) packet.dest_addr;


        if (dest == nodeinfo.nodenumber) {
                printf("Pkt received: src=%zu, dest=%zu\n", packet.src_addr,
                                                          packet.dest_addr);
                network_upto_transport(&link, packet.msg.data, length );
        }
        else { 
                printf("src=%d, dest=%d\n", src, dest);
                printf("Fwd packet...\n");
                link = routing_table[nodeinfo.nodenumber][dest];
                system("echo bam");
                network_downto_datalink(link, packet, length);
        }
}




/*****************************************************************************
 * Data Link Layer 
 *
 * If your assignment is to develop data link layer protocol, then the codes
 * are inserted in the following functions. You may create other 
 * supporting/utilite function if required.
 * 
 * Note: this layer is to create a Frame, 
 *       Frame <- (data link layer header) + (Packet)
 *
 *
 ******************************************************************************/

void network_downto_datalink(int link, Packet_T packet, size_t length)
{
        Frame_T frame;

        memcpy(&frame.payload, &packet, sizeof(packet));
        frame.kind      = DL_DATA;
        frame.checksum  = 0;
        frame.seq       = seqno;
        FILE*           f           = fopen("logfile", "a");

        fprintf(f, "network_dt_datalink():\nsrc_addr=%d\ndest_addr=%d\nkind=%d\n"
                        "chsum=%d\nseq=%d\n"
                        "msg=%s\n\n", 
                        frame.payload.src_addr,
                        frame.payload.dest_addr,
                        frame.kind,
                        frame.checksum,
                        frame.seq,
                        frame.payload.msg.data);
        fclose(f);

        length = FRAME_SIZE(frame);
        frame.checksum  = CNET_ccitt((unsigned char *)&frame, (int)length);
        CHECK(CNET_write_physical(link, (void*)&frame, &length));
}

static void frame_arrived(CnetEvent ev, CnetTimerID timer, CnetData data)
{
        FILE*           f           = fopen("logfile", "a");
        Frame_T         frame;
        int             link, kind;
        size_t          length;

        length = sizeof(Frame_T);

        CHECK(CNET_read_physical(&link, (char*)&frame, &length));

        fprintf(f, "frame_arrived():\nsrc_addr=%d\ndest_addr=%d\nkind=%d\n"
                        "chsum=%d\nseq=%d\n"
                        "msg=%s\n\n", 
                        frame.payload.src_addr,
                        frame.payload.dest_addr,
                        frame.kind,
                        frame.checksum,
                        frame.seq,
                        frame.payload.msg.data);
        fclose(f);

        kind = (int) frame.kind; 

        printf("fr arr from src=%d, dest=%d\n", frame.payload.src_addr,
                                                       frame.payload.dest_addr);
        if (kind == DL_DATA) {
                datalink_upto_network(link, frame.payload, length );
        } 
        else {
                printf("ACK received, seq=%d\n", frame.seq);
        }
}






/*************************************************************************
 *
 *      Initialisation stuffs
 *
 *************************************************************************/

void reboot_host()
{
    /*  Indicate our interest in certain cnet events */
        printf("Initialising current node as a host.\n");
    
    /* When everything is working properly, you can uncomment this to
     * let the application layer generate the messages for you 
     */
        /*CHECK(CNET_set_handler( EV_APPLICATIONREADY, application_ready, 0));*/

        /* right now, read from the keyboard only */
        CHECK(CNET_set_handler( EV_KEYBOARDREADY, keyboard, 0));
        CHECK(CNET_set_handler( EV_PHYSICALREADY, frame_arrived, 0));

    /* CNET_enable_application(ALLNODES);*/
        prompt(1);
}

void reboot_router()
{

    /*  Indicate our interest in certain cnet events */
    printf("Initialising current node as a router.\n");

    /* NOTE: set any events that is required in your assignment. 
     * current for this simple working example, there isn't any routers
     */
}

void reboot_node(CnetEvent ev, CnetTimerID timer, CnetData data)
{
        
    printf("Hello world, this is node %d\n", nodeinfo.nodenumber);

    /* CODE to define HOST from ROUTER */
    /*if (nodeinfo.nlinks == 1)*/
        reboot_host();
    /*else if (nodeinfo.nlinks > 1)*/
        /*reboot_router();*/
    /*else*/
        /*printf("Is there anybody out there ?\n");*/
}

