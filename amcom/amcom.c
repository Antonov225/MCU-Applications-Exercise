#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "amcom.h"

/// Start of packet character
const uint8_t  AMCOM_SOP         = 0xA1;
const uint16_t AMCOM_INITIAL_CRC = 0xFFFF;

static uint16_t AMCOM_UpdateCRC(uint8_t byte, uint16_t crc)
{
	byte ^= (uint8_t)(crc & 0x00ff);
	byte ^= (uint8_t)(byte << 4);
	return ((((uint16_t)byte << 8) | (uint8_t)(crc >> 8)) ^ (uint8_t)(byte >> 4) ^ ((uint16_t)byte << 3));
}


void AMCOM_InitReceiver(AMCOM_Receiver* receiver, AMCOM_PacketHandler packetHandlerCallback, void* userContext) {

	receiver->payloadCounter = 0;
	receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
	receiver->packetHandler = packetHandlerCallback;
	receiver->userContext = userContext;
}

size_t AMCOM_Serialize(uint8_t packetType, const void* payload, size_t payloadSize, uint8_t* destinationBuffer) {

    //Check for improper values
	if(destinationBuffer == NULL && payloadSize != 0){
	    return -1;
	}
	if(payload == NULL && payloadSize != 0){
	    return -1;
	}
	if((payload != NULL) && (payloadSize == 0)){
	    return -1;
	}
	
	AMCOM_PacketHeader * header = (AMCOM_PacketHeader*) destinationBuffer;
	
	//Construct header
	header->sop = 0xA1;
	header->type = packetType;
    header->length= payloadSize;
	
	//Calculate CRC based on packetType and payloadSize
	uint16_t crc = 0xFFFF;
	crc = AMCOM_UpdateCRC(packetType, crc);
	crc = AMCOM_UpdateCRC(payloadSize, crc);
	
	//When no payload
	if(payloadSize == 0){
	    header->crc = crc;
	    return 5;
	}
	
	//When payload with proper size
	else if(payloadSize > 0 && payloadSize <= 200){
	    
	    //Copy payload to buffer
	    memcpy(destinationBuffer+5, payload, payloadSize);
	    
	    //Calculate CRC for based on payload content
	    for(int i = 0; i < payloadSize; i++){
	        crc = AMCOM_UpdateCRC(*((uint8_t*)payload+i), crc);
	    }
	    
        //Add final CRC to header
	    header->crc = crc;
	    return 5 + payloadSize;
	}
    else{
	    return -1;
    }
}

void AMCOM_Deserialize(AMCOM_Receiver* receiver, const void* data, size_t dataSize) {

    //Main state loop
    for(int i = 0; i< dataSize; i++){
        //Transition to GOT_SOP
        if(receiver->receivedPacketState == AMCOM_PACKET_STATE_EMPTY){
            if(*((uint8_t*)data+i) == 0xA1){
               receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_SOP;
               receiver->receivedPacket.header.sop = 0xA1;
                continue;
            }
            else{
                continue;
            }
       }
       
       //Transition to GOT_TYPE 
       if(receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_SOP){
           receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_TYPE;
           receiver->receivedPacket.header.type = *((uint8_t*)data+i);
           continue;
       }
       
       //Transition to GOT_LENGTH or EMPTY
       if(receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_TYPE){
           if(*((uint8_t*)data+i) <= 200 && *((uint8_t*)data+i) >= 0){
               receiver->receivedPacket.header.length = *((uint8_t*)data+i);
               receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_LENGTH;
               continue;
           }
           else{
               //Transition to EMPTY
               receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
               continue;
           }
       }
       
       //Transition to GOT_CRC_LO
       if(receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_LENGTH){
            receiver->receivedPacket.header.crc = *((uint8_t*)data+i);
            receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_CRC_LO;
            continue;
       }
       
       //Transition to GOT_WHOLE_PACKET or GETTING_PAYLOAD
       if(receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_CRC_LO){
           receiver->receivedPacket.header.crc |= *((uint8_t*)data+i) << 8;
           if(receiver->receivedPacket.header.length == 0){ //LENGTH == 0
               receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_WHOLE_PACKET;
               //continue;
           }
           if(receiver->receivedPacket.header.length > 0){ //LENGTH > 0
               receiver->receivedPacketState = AMCOM_PACKET_STATE_GETTING_PAYLOAD;
               receiver->payloadCounter =0;// TO DO: sprawdzic czy powinno byc resetowanie
               continue;
           }
       }
       
       //Transition to GOT_WHOLE_PACKET from GETTING_PAYLOAD
       if(receiver->receivedPacketState == AMCOM_PACKET_STATE_GETTING_PAYLOAD){
           if(receiver->payloadCounter < receiver->receivedPacket.header.length -1){
           //Write data except last chunk to allow executing transition when writing last chunk
               receiver->receivedPacket.payload[receiver->payloadCounter] = *((uint8_t*)data+i);
               receiver->payloadCounter++;
               continue;
           }
           //Execute transition
           receiver->receivedPacket.payload[receiver->payloadCounter] = *((uint8_t*)data+i);
           receiver->receivedPacketState = AMCOM_PACKET_STATE_GOT_WHOLE_PACKET;
       }
       
       //GOT_WHOLE_PACKET
       if(receiver->receivedPacketState == AMCOM_PACKET_STATE_GOT_WHOLE_PACKET){
            //Calculate CRC based on type, length, payload
            uint16_t crc = 0xFFFF; 
            crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.type, crc);
        	crc = AMCOM_UpdateCRC(receiver->receivedPacket.header.length, crc);

        	if(receiver->receivedPacket.header.length >0){
        	    for(int j = 0; j< receiver->receivedPacket.header.length; j++){
        	        crc = AMCOM_UpdateCRC(receiver->receivedPacket.payload[j], crc);
        	    }
            }
            
            //Check CRC, handle userContext, transition back to EMPTY
            if(crc == receiver->receivedPacket.header.crc){
                receiver->receivedPacketState = AMCOM_PACKET_STATE_EMPTY;
                (*receiver->packetHandler)(&receiver->receivedPacket, receiver->userContext);
            }
       }
    }  
}
