#ifndef __interface_h
#define __interface_h

//#define __PACKED		__attribute__((packed, aligned(1)))
#define __PACKED

typedef unsigned int	u32;
typedef unsigned short	u16;
typedef unsigned char	u8;

#define MAC_PDU_MAX_LENGTH			(768)	// 24 byte * 32 phy slot
#define PHY_HEADER_LENGTH			12
#define PHY_PDU_MAX_LENGTH			(MAC_PDU_MAX_LENGTH + PHY_HEADER_LENGTH)

typedef enum
{
	ED_UPLINK_PACKET	= 0,
	ED_DOWNLINK_ACK,
	ED_DOWNLINK_PACKET,
	ED_UPLINK_ACK,
	ED_KEEP_ALIVE,
	ED_RESET_REQUEST,
	ED_MAC_READY_INDICATION,
	ED_SLEEP_REQEUST,
	ED_SLEEP_RESPONSE,
} HOST_INTERFACE_PACKET_TYPE;


#pragma pack(push, 1) 
typedef struct
{
	u32		type;
	u32		device_address;
	u32		UTC_time;
	u32		frequency;
	u8		bandwidth;
	u8		repetition;
	double	rssi;
	double	snr;
	u16		sequence_number;
	u16		payload_length;
	u8		payload[PHY_PDU_MAX_LENGTH];
} MAC_UPLINK_PACKET_TYPE;

typedef struct
{
	u32		type;				// 4
	u32		device_address;		// 4
	u32		UTC_time;			// 4
	u32		frequency;			// 4
	u32		bandwidth;			// 4
	u32		repetition;			// 4
	u32		ack_include;		// 4
	u16		payload_length;		// 2
	u8		payload[PHY_PDU_MAX_LENGTH];
} MAC_DOWNLINK_PACKET_TYPE;
#pragma pack(pop) 

#pragma pack(push, 1)
typedef struct
{
	u32		tick_count;
	double	pressure;
} WATER_METER_TYPE;


typedef struct
{
	u32 type;
	u32 device_address;
	u32 frequency;
	u32 bandwidth;
	u32 repetition;
} MAC_READY_INDICATION_TYPE;

typedef struct
{
	u32 type;
	u32 wakeup_type;
	u32 UTC_time;
} MAC_SLEEP_REQUEST_TYPE;

typedef struct
{
	u32 type;
	u32 confirm;
} MAC_SLEEP_RESPONSE_TYPE;
#pragma pack(pop)

#define MIN_UPLINK_PACKET_SIZE	(sizeof(MAC_UPLINK_PACKET_TYPE) - PHY_PDU_MAX_LENGTH)

#endif