#include <mictcp.h>
#include <api/mictcp_core.h>

int next_seq_num = 0;
int next_expected = 0;
int nb_perte = 0;
int nb_msg = 0;


/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(30);

   return result;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   return 0;
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    	printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    	mic_tcp_pdu pdu;
	mic_tcp_sock_addr addr;
	int size;
	float perte_accept = 30;
	//float prct_perte;
   
	//pdu.header.source_port = htons(addr.port);
	//pdu.header.dest_port = pdu.header.source_port;
	pdu.header.seq_num = next_seq_num;
	pdu.payload.data = mesg;
    	pdu.payload.size = mesg_size;
    	size = IP_send(pdu, addr);
	nb_msg ++;
    	mic_tcp_pdu ack;
    	mic_tcp_sock_addr addr_ack;
    	int recvResult;
    	recvResult = IP_recv(&ack, &addr_ack, 50);
    	sleep(1);
	
	if (recvResult < 0 || ack.header.ack_num != (pdu.header.seq_num+1)%2) {
		nb_perte ++;
	}
	
	while (recvResult < 0 || ack.header.ack_num != (pdu.header.seq_num+1)%2) {
        	printf("next_seq_num = %d et ack_num = %d\n", (pdu.header.seq_num+1)%2, ack.header.ack_num);
        	float prct_perte = (100*nb_perte)/nb_msg;
        	printf("nb_msg : %d\n", nb_msg);
        	printf("nb_perte : %d\n", nb_perte);
        	printf("prct_perte : %f\n", prct_perte);
        	if (prct_perte <= perte_accept) {
        		printf("perte, non renvoi du PDU\n");
        		break;
        	} else {
			size = IP_send(pdu, addr);
        		recvResult = IP_recv(&ack, &addr_ack, 50);
        		printf("après renvoi : next_seq_num = %d et ack_num = %d\n", (pdu.header.seq_num+1)%2, ack.header.ack_num);
        		sleep(1);
        	}
	}

    	next_seq_num = (next_seq_num+1)%2;
	return size;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_payload payload;
   	payload.data = mesg;
   	payload.size = max_mesg_size;
 	int effective_size = app_buffer_get(payload);
    return effective_size;
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    return 0;
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
	mic_tcp_pdu ack;
    ack.header.ack = 1;
    ack.payload.size = 0;
    if(pdu.header.seq_num == next_expected) {
	app_buffer_put(pdu.payload);
        next_expected = (next_expected+1)%2;
 	}
    ack.header.ack_num = next_expected;
    printf("valeur de num_ack à l'envoi : %d\n", ack.header.ack_num);
    IP_send(ack, addr);
}
