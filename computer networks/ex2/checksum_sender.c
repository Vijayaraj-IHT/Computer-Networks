#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHANNEL_FILE "bytecount_channel.txt"

#define COUNT_BITS 16
#define CHECKSUM_BITS 8
#define BUFFER_SIZE 1000

/* ---------------- Basic Binary Utilities ---------------- */

void decimalToBinary(unsigned int value, int bits, char binary[]) {
    int i;
    binary[bits] = '\0';
    for (i = bits - 1; i >= 0; i--) {
        binary[i] = (value % 2) + '0';
        value = value / 2;
    }
}

unsigned int binaryToDecimal(const char binary[], int bits) {
    unsigned int value = 0;
    int i;
    for (i = 0; i < bits; i++) {
        value = value * 2 + (binary[i] - '0');
    }
    return value;
}

int ipToBinary(const char *ip, char binary[]) {
    unsigned int a, b, c, d;
    char temp[9];

    if (sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d) != 4) {
        return 0;
    }
    if (a > 255 || b > 255 || c > 255 || d > 255) {
        return 0;
    }

    binary[0] = '\0';
    decimalToBinary(a, 8, temp); strcat(binary, temp);
    decimalToBinary(b, 8, temp); strcat(binary, temp);
    decimalToBinary(c, 8, temp); strcat(binary, temp);
    decimalToBinary(d, 8, temp); strcat(binary, temp);

    return 1;
}

int macToBinary(const char *mac, char binary[]) {
    unsigned int b1, b2, b3, b4, b5, b6;
    char temp[9];

    if (sscanf(mac, "%x:%x:%x:%x:%x:%x", &b1, &b2, &b3, &b4, &b5, &b6) != 6) {
        return 0;
    }
    if (b1 > 255 || b2 > 255 || b3 > 255 || b4 > 255 || b5 > 255 || b6 > 255) {
        return 0;
    }

    binary[0] = '\0';
    decimalToBinary(b1, 8, temp); strcat(binary, temp);
    decimalToBinary(b2, 8, temp); strcat(binary, temp);
    decimalToBinary(b3, 8, temp); strcat(binary, temp);
    decimalToBinary(b4, 8, temp); strcat(binary, temp);
    decimalToBinary(b5, 8, temp); strcat(binary, temp);
    decimalToBinary(b6, 8, temp); strcat(binary, temp);

    return 1;
}

/* ---------------- Checksum (Sum Addition) Engine ---------------- */

/*
 * Adds all bytes together and takes Modulo-256 to generate an 8-bit checksum.
 */
void calculateChecksum(const char *data, char checksum[]) {
    int length;
    int sum;
    int i;
    char byteString[9];
    unsigned int byteValue;

    length = strlen(data);
    sum = 0;

    /* Process the binary string byte-by-byte (every 8 bits) */
    for (i = 0; i < length; i = i + 8) {
        strncpy(byteString, data + i, 8);
        byteString[8] = '\0';

        byteValue = binaryToDecimal(byteString, 8);
        sum = sum + (int)byteValue;
    }

    /* Keep only the lowest 8 bits (Modulo 256) */
    sum = sum % 256;

    decimalToBinary((unsigned int)sum, CHECKSUM_BITS, checksum);
}

/* ---------------- OSI Sender Layers ---------------- */

void applicationLayer(unsigned char character, char pdu[]) {
    decimalToBinary(character, 8, pdu);
    printf(" [APP LAYER]   Character '%c' -> %s\n", character, pdu);
}

void transportLayer(char pdu[], unsigned int sourcePort, unsigned int destinationPort) {
    char sourcePortBinary[17];
    char destinationPortBinary[17];
    char temporary[BUFFER_SIZE];

    decimalToBinary(sourcePort, 16, sourcePortBinary);
    decimalToBinary(destinationPort, 16, destinationPortBinary);

    sprintf(temporary, "%s%s%s", sourcePortBinary, destinationPortBinary, pdu);
    strcpy(pdu, temporary);

    printf(" [TRANS LAYER] Added source/destination ports: %d bits\n", (int)strlen(pdu));
}

int networkLayer(char pdu[], const char *sourceIP, const char *destinationIP) {
    char sourceIPBinary[33];
    char destinationIPBinary[33];
    char temporary[BUFFER_SIZE];

    if (!ipToBinary(sourceIP, sourceIPBinary) || !ipToBinary(destinationIP, destinationIPBinary)) {
        printf("Invalid IP address.\n");
        return 0;
    }

    sprintf(temporary, "%s%s%s", destinationIPBinary, sourceIPBinary, pdu);
    strcpy(pdu, temporary);

    printf(" [NET LAYER]   Added destination/source IPs: %d bits\n", (int)strlen(pdu));
    return 1;
}

int dataLinkLayer(char pdu[], const char *sourceMAC, const char *destinationMAC) {
    char sourceMACBinary[49];
    char destinationMACBinary[49];
    char protectedData[BUFFER_SIZE];
    char checksum[CHECKSUM_BITS + 1];
    char countBinary[COUNT_BITS + 1];
    char finalFrame[BUFFER_SIZE];
    int totalBits;
    unsigned int totalBytes;

    if (!macToBinary(sourceMAC, sourceMACBinary) || !macToBinary(destinationMAC, destinationMACBinary)) {
        printf("Invalid MAC address.\n");
        return 0;
    }

    /* Combine MACs + IPs + Ports + Data */
    sprintf(protectedData, "%s%s%s", destinationMACBinary, sourceMACBinary, pdu);
    printf(" [LINK LAYER]  Added MAC headers: %d bits\n", (int)strlen(protectedData));

    /* Calculate 8-bit Checksum over headers and payload */
    calculateChecksum(protectedData, checksum);
    printf(" [LINK LAYER]  Calculated 8-bit Checksum: %s (%u)\n", checksum, binaryToDecimal(checksum, 8));

    /* Append Checksum */
    strcat(protectedData, checksum);
    printf(" [LINK LAYER]  PDU + Checksum (%d bits):\n               %s\n", (int)strlen(protectedData), protectedData);

    /* Calculate total frame size (Count field + Protected Data) */
    totalBits = COUNT_BITS + (int)strlen(protectedData);
    totalBytes = (unsigned int)(totalBits / 8);

    decimalToBinary(totalBytes, COUNT_BITS, countBinary);

    /* Prepend Byte Count header */
    sprintf(finalFrame, "%s%s", countBinary, protectedData);
    strcpy(pdu, finalFrame);

    printf(" [LINK LAYER]  Byte count: %u bytes\n", totalBytes);
    printf(" [LINK LAYER]  Count field: %s\n", countBinary);
    printf(" [LINK LAYER]  Final frame (%d bits):\n               |COUNT=%s|%s\n", (int)strlen(pdu), countBinary, pdu + COUNT_BITS);

    return 1;
}

/* ---------------- Main Function ---------------- */

int main(void) {
    char message[2000];
    printf("\nMessage to send : ");
    scanf("%s",&message);
    unsigned int sourcePort = 54321;
    unsigned int destinationPort = 8080;
    char sourceIP[] = "192.168.1.10";
    char destinationIP[] = "142.250.190.78";
    char sourceMAC[] = "AA:BB:CC:11:22:33";
    char destinationMAC[] = "DD:EE:FF:44:55:66";

    FILE *channel;
    int i;
    char pdu[BUFFER_SIZE];

    channel = fopen(CHANNEL_FILE, "w");
    if (channel == NULL) {
        printf("Cannot open channel file.\n");
        return 1;
    }

    printf("========================================================\n");
    printf("     OSI SENDER - BYTE COUNT PROTOCOL + CHECKSUM        \n");
    printf("========================================================\n");

    for (i = 0; message[i] != '\0'; i++) {
        pdu[0] = '\0';
        printf("\n--- Encapsulating character '%c' ---\n", message[i]);

        applicationLayer((unsigned char)message[i], pdu);
        transportLayer(pdu, sourcePort, destinationPort);

        if (!networkLayer(pdu, sourceIP, destinationIP)) {
            fclose(channel);
            return 1;
        }
        if (!dataLinkLayer(pdu, sourceMAC, destinationMAC)) {
            fclose(channel);
            return 1;
        }

        fprintf(channel, "%s\n", pdu);
        printf(" [CHANNEL]     Frame written to %s\n", CHANNEL_FILE);
    }

    fclose(channel);
    printf("\nSender completed successfully.\n");
    return 0;
}
