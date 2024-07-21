/*
 * clone_master.cpp
 *
 *  Created on: Mar 30, 2022
 *      Author: asharma
 */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <unistd.h>

/*******************************************************************************
Computes the incoming clone packet checksum.
*******************************************************************************/
class Clone{
private:
	static const int BUF_SIZE = 40;
	static const int CLONE_READ_PORT  = 51112;
	static const int CLONE_WRITE_PORT = 51111;
	static const int CLONE_PKT_VER    = 0xB30D;

	union packet_u{
		struct payload_s{
				uint16_t version;
				uint16_t checksum;
				uint16_t cloning_enabled;
				uint16_t switching_mode;
				uint16_t switching_type;
				uint16_t warm_standby;
				uint16_t cloning_change_flags;
				uint16_t alarm_minor_mask;
				uint16_t alarm_major_mask;
				uint16_t suppression_mask;
				uint16_t output_high_threshold;
				uint16_t output_low_threshold;
				uint16_t burst_timeout;
				uint16_t burst_threshold;
				uint16_t monitor_frequency;
				uint16_t power_monitor_mode;
				uint16_t tx_power_up_delay;
				uint16_t tx_power_up_state;
				uint16_t tx_enable;
				uint16_t temperature_shutdown;
		}payload;
		uint8_t buf[BUF_SIZE];
	}rx_packet, tx_packet;


public:
	Clone(){}

	int setup_server(void){
		int sfd = 0;
		struct sockaddr_in	servaddr;

		sfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(sfd < 0){
			perror("socket error;");
		}

		memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family      = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port        = htons(CLONE_READ_PORT);

		if( bind(sfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ){
			perror("socket bind");
		}
		return sfd;
	}

	int setup_client(struct sockaddr_in	&client_addr){

		int cfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(cfd < 0){
			perror("socket error;");
		}

		memset(&client_addr, 0, sizeof(client_addr));
		client_addr.sin_family      = AF_INET;
		client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		client_addr.sin_port        = htons(CLONE_WRITE_PORT);

		return cfd;
	}

	uint16_t compute_read_cksum(int recv_len){
	  uint8_t *ptr = rx_packet.buf;
	  uint16_t len = recv_len;
		uint16_t cksum = 0, idx;

	  /* Compute checksum for version and the rest of the data */
	  for(idx=0; idx < len; idx++) {
	  	if((idx == 2) || (idx == 3)){
	  		/* Skip the checksum since it was set to zero by the sender before
	  		 computing the checksum. */
	  		continue;
	  	}else{
	  		cksum += (ptr[idx] & 0x00FF);
	  	}
	  }
	  return cksum;
	}

	uint16_t compute_write_cksum(void){
	  uint16_t cksum = 0, idx;
	  uint8_t buf[BUF_SIZE] = {0};
	  int len = sizeof(tx_packet.buf);

	  memcpy(buf, (uint8_t *)&tx_packet.payload, len);

	  for(idx=0; idx<len; idx++) {
	    cksum += (buf[idx] & (0x00FF));
	  }
	  return cksum;
	}

	void clear_buffer(void){
		memset(rx_packet.buf, 0, sizeof(rx_packet.buf));
	}

	int get_packet(int sfd, struct sockaddr_in &client_addr){
		int recv_len = 0;
		socklen_t client_len = sizeof(client_addr); /* Not used elsewhere */

		if ((recv_len = recvfrom(sfd, rx_packet.buf, BUF_SIZE, 0, \
				(struct sockaddr *)&client_addr, &client_len)) == -1){
			perror("recvfrom()");
		}
		return recv_len;
	}

	void send_packet(int cfd, struct sockaddr_in client_addr){
		socklen_t client_len = sizeof(client_addr);
		int send_len         = sizeof(tx_packet.payload);

		if ((sendto(cfd, tx_packet.buf, send_len, 0, (struct sockaddr *)&client_addr, client_len)) == -1){
			perror("sendto()");
		}
	}

	void build_packet(void){

		tx_packet.payload.version              = CLONE_PKT_VER;
		tx_packet.payload.cloning_enabled      = 1;
		tx_packet.payload.switching_mode       = 0;
		tx_packet.payload.switching_type       = 0;
		tx_packet.payload.warm_standby         = 0;
		tx_packet.payload.cloning_change_flags = 0;
		tx_packet.payload.alarm_minor_mask     = 0xffff;
		tx_packet.payload.alarm_major_mask     = 0x8888;
		tx_packet.payload.suppression_mask     = 0xaaaa;
		tx_packet.payload.output_high_threshold= 6000;
		tx_packet.payload.output_low_threshold = 1500;
		tx_packet.payload.burst_timeout        = 500;
		tx_packet.payload.burst_threshold      = 1500;
		tx_packet.payload.monitor_frequency    = 5650;
		tx_packet.payload.power_monitor_mode   = 0;
		tx_packet.payload.tx_power_up_delay    = 0;
		tx_packet.payload.tx_power_up_state    = 1;
		tx_packet.payload.tx_enable            = 1;
		tx_packet.payload.temperature_shutdown = 1;

	}

	void dump(struct sockaddr_in client_addr, int recv_len, uint16_t checksum){
		if(recv_len == sizeof(rx_packet)){
			if(rx_packet.payload.checksum == checksum){
				fprintf(stderr,"client IP:%s\n",inet_ntoa(client_addr.sin_addr));
				fprintf(stderr,"client Port:%d\n",client_addr.sin_port);
				fprintf(stderr,"checksum=%d\n",checksum);
				fprintf(stderr,"%s",rx_packet.buf);
				fprintf(stderr,"packet length=%d",recv_len);
			}else{
				fprintf(stderr,"payload checksum=%d, checksum=%d\n",
						rx_packet.payload.checksum,checksum);
			}
		}
	}
};

int main(void){

	Clone cloning = Clone();
	int sfd = cloning.setup_server();

	struct sockaddr_in client_addr;
	int cfd = cloning.setup_client(client_addr);
	cloning.build_packet();

	while(1){
		/* Tx portion */
		uint16_t tx_checksum = cloning.compute_write_cksum();
		cloning.send_packet(cfd, client_addr);

#if(0)
		/* Rx portion */
		cloning.clear_buffer();

		struct sockaddr_in client_addr = {0};
		int recv_len = cloning.get_packet(sfd, client_addr);

		int checksum = cloning.compute_read_cksum(recv_len);
		cloning.dump(client_addr, recv_len, checksum);
#endif
		sleep(2);
		/* Include sendto also */
	}
	return 0;
}
