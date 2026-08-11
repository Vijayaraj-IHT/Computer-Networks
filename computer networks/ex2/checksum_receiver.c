#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHANNEL_FILE "bytecount_channel.txt"

#define COUNT_BITS 16
#define CHECKSUM_BITS 8
#define MAC_HEADER_BITS 96
#define IP_HEADER_BITS 64
#define PORT_HEADER_BITS 32
#define DATA_BITS 8

#define BUFFER_SIZE 1000
#define MESSAGE_SIZE 256

/* ---------------- Basic Binary Utilities ---------------- */

unsigned int binaryToDecimal(const char binary[], int bits) {
    unsigned int value = 0;
    int i;
    for (i = 0; i < bits; i++) {
        value = value * 2 + (binary[i] - '0');
    }
    return value;
}

void decimalToBinary(unsigned int value, int bits, char binary[]) {
    int i;
    binary[bits] = '\0';
    for (i = bits - 1; i >= 0; i--) {
        binary[i] = (value % 2) + '0';
        value = value / 2;
    }
}

int isBinaryString(const char binary[]) {
    int i;
    for (i = 0; binary[i] != '\0'; i++) {
        if (binary[i] != '0' && binary[i] != '1') {
            return 0;
        }
    }
    return 1;
}

void binaryToIP(const char binary[], char ip[]) {
    unsigned int a = binaryToDecimal(binary, 8);
    unsigned int b = binaryToDecimal(binary + 8, 8);
    unsigned int c = binaryToDecimal(binary + 16, 8);
    unsigned int d = binaryToDecimal(binary + 24, 8);
    sprintf(ip, "%u.%u.%u.%u", a, b, c, d);
}

void binaryToMAC(const char binary[], char mac[]) {
    unsigned int b1 = binaryToDecimal(binary, 8);
    unsigned int b2 = binaryToDecimal(binary + 8, 8);
    unsigned int b3 = binaryToDecimal(binary + 16, 8);
    unsigned int b4 = binaryToDecimal(binary + 24, 8);
    unsigned int b5 = binaryToDecimal(binary + 32, 8);
    unsigned int b6 = binaryToDecimal(binary + 40, 8);
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", b1, b2, b3, b4, b5, b6);
}

/* ---------------- Checksum Verification Engine ---------------- */

void calculateChecksum(const char *data, char checksum[]) {
    int length = strlen(data);
    int sum = 0;
    int i;
    char byteString[9];
    unsigned int byteValue;

    for (i = 0; i < length; i = i + 8) {
        strncpy(byteString, data + i, 8);
        byteString[8] = '\0';
        byteValue = binaryToDecimal(byteString, 8);
        sum = sum + (int)byteValue;
    }

    sum = sum % 256;
    decimalToBinary((unsigned int)sum, CHECKSUM_BITS, checksum);
}

int verifyChecksum(const char *dataWithChecksum) {
    int totalLength;
    int dataLength;
    char dataOnly[BUFFER_SIZE];
    char receivedChecksum[CHECKSUM_BITS + 1];
    char calculatedChecksum[CHECKSUM_BITS + 1];

    totalLength = strlen(dataWithChecksum);
    if (totalLength < CHECKSUM_BITS) {
        return 0;
    }

    dataLength = totalLength - CHECKSUM_BITS;

    /* Separate data from received checksum */
    strncpy(dataOnly, dataWithChecksum, dataLength);
    dataOnly[dataLength] = '\0';

    strncpy(receivedChecksum, dataWithChecksum + dataLength, CHECKSUM_BITS);
    receivedChecksum[CHECKSUM_BITS] = '\0';

    /* Recalculate checksum over the data portion */
    calculateChecksum(dataOnly, calculatedChecksum);

    printf(" [LINK LAYER]  Received Checksum:   %s (%u)\n", receivedChecksum, binaryToDecimal(receivedChecksum, 8));
    printf(" [LINK LAYER]  Calculated Checksum: %s (%u)\n", calculatedChecksum, binaryToDecimal(calculatedChecksum, 8));

    if (strcmp(receivedChecksum, calculatedChecksum) == 0) {
        return 1; /* PASS */
    } else {
        return 0; /* FAIL */
    }
}

/* ---------------- OSI Receiver Layers ---------------- */

int dataLinkReceiver(char pdu[]) {
    char countBinary[COUNT_BITS + 1];
    char frameBody[BUFFER_SIZE];
    char destinationMACBinary[49];
    char sourceMACBinary[49];
    char destinationMAC[32];
    char sourceMAC[32];
    char temporary[BUFFER_SIZE];

    int actualBits;
    unsigned int declaredBytes;
    int declaredBits;
    int bodyLength;
    int minimumBodyLength;
    int withoutChecksumLength;
    int remainingLength;

    actualBits = strlen(pdu);
    printf(" [LINK LAYER]  Raw frame (%d bits):\n               %s\n", actualBits, pdu);

    if (!isBinaryString(pdu) || actualBits < COUNT_BITS) {
        printf(" [LINK LAYER]  Corrupted frame or too short. DROP\n");
        return 0;
    }

    /* Read Byte Count header */
    strncpy(countBinary, pdu, COUNT_BITS);
    countBinary[COUNT_BITS] = '\0';

    declaredBytes = binaryToDecimal(countBinary, COUNT_BITS);
    declaredBits = declaredBytes * 8;

    printf(" [LINK LAYER]  Count field: %s\n", countBinary);
    printf(" [LINK LAYER]  Declared size: %u bytes (%d bits) | Actual size: %d bits\n", declaredBytes, declaredBits, actualBits);

    if (declaredBits != actualBits) {
        printf(" [LINK LAYER]  Byte-count verification: FAIL\n");
        return 0;
    }
    printf(" [LINK LAYER]  Byte-count verification: PASS\n");

    /* Extract frame body */
    bodyLength = actualBits - COUNT_BITS;
    strncpy(frameBody, pdu + COUNT_BITS, bodyLength);
    frameBody[bodyLength] = '\0';

    minimumBodyLength = MAC_HEADER_BITS + IP_HEADER_BITS + PORT_HEADER_BITS + DATA_BITS + CHECKSUM_BITS;
    if (bodyLength < minimumBodyLength) {
        printf(" [LINK LAYER]  Frame body too short. DROP\n");
        return 0;
    }

    /* Verify Checksum */
    if (!verifyChecksum(frameBody)) {
        printf(" [LINK LAYER]  Checksum verification: FAIL - FRAME DROPPED\n");
        return 0;
    }
    printf(" [LINK LAYER]  Checksum verification: PASS\n");

    /* Strip Checksum */
    withoutChecksumLength = bodyLength - CHECKSUM_BITS;
    frameBody[withoutChecksumLength] = '\0';

    /* Decode and display MAC addresses */
    strncpy(destinationMACBinary, frameBody, 48); destinationMACBinary[48] = '\0';
    strncpy(sourceMACBinary, frameBody + 48, 48); sourceMACBinary[48] = '\0';
    binaryToMAC(destinationMACBinary, destinationMAC);
    binaryToMAC(sourceMACBinary, sourceMAC);

    printf(" [LINK LAYER]  Destination MAC: %s | Source MAC: %s\n", destinationMAC, sourceMAC);

    /* Strip 96-bit MAC header */
    remainingLength = withoutChecksumLength - MAC_HEADER_BITS;
    strncpy(temporary, frameBody + MAC_HEADER_BITS, remainingLength);
    temporary[remainingLength] = '\0';
    strcpy(pdu, temporary);

    printf(" [LINK LAYER]  Stripped MACs and Checksum: %d bits remain\n", (int)strlen(pdu));
    return 1;
}

int networkReceiver(char pdu[]) {
    char destinationIPBinary[33];
    char sourceIPBinary[33];
    char destinationIP[32];
    char sourceIP[32];
    char temporary[BUFFER_SIZE];
    int length = strlen(pdu);
    int remainingLength;

    if (length < IP_HEADER_BITS) {
        printf(" [NET LAYER]   Missing IP headers. DROP\n");
        return 0;
    }

    strncpy(destinationIPBinary, pdu, 32); destinationIPBinary[32] = '\0';
    strncpy(sourceIPBinary, pdu + 32, 32); sourceIPBinary[32] = '\0';
    binaryToIP(destinationIPBinary, destinationIP);
    binaryToIP(sourceIPBinary, sourceIP);

    printf(" [NET LAYER]   Destination IP: %s | Source IP: %s\n", destinationIP, sourceIP);

    remainingLength = length - IP_HEADER_BITS;
    strncpy(temporary, pdu + IP_HEADER_BITS, remainingLength);
    temporary[remainingLength] = '\0';
    strcpy(pdu, temporary);

    printf(" [NET LAYER]   Stripped IP headers: %d bits remain\n", (int)strlen(pdu));
    return 1;
}

int transportReceiver(char pdu[]) {
    char sourcePortBinary[17];
    char destinationPortBinary[17];
    char temporary[BUFFER_SIZE];
    unsigned int sourcePort, destinationPort;
    int length = strlen(pdu);
    int remainingLength;

    if (length < PORT_HEADER_BITS) {
        printf(" [TRANS LAYER] Missing port headers. DROP\n");
        return 0;
    }

    strncpy(sourcePortBinary, pdu, 16); sourcePortBinary[16] = '\0';
    strncpy(destinationPortBinary, pdu + 16, 16); destinationPortBinary[16] = '\0';
    sourcePort = binaryToDecimal(sourcePortBinary, 16);
    destinationPort = binaryToDecimal(destinationPortBinary, 16);

    printf(" [TRANS LAYER] Source Port: %u | Destination Port: %u\n", sourcePort, destinationPort);

    remainingLength = length - PORT_HEADER_BITS;
    strncpy(temporary, pdu + PORT_HEADER_BITS, remainingLength);
    temporary[remainingLength] = '\0';
    strcpy(pdu, temporary);

    printf(" [TRANS LAYER] Stripped ports: %d bits remain\n", (int)strlen(pdu));
    return 1;
}

int applicationReceiver(char pdu[], char recoveredMessage[], int *messageIndex) {
    unsigned int value;
    char recoveredCharacter;

    if ((int)strlen(pdu) < DATA_BITS) {
        printf(" [APP LAYER]   Missing application data.\n");
        return 0;
    }

    value = binaryToDecimal(pdu, DATA_BITS);
    recoveredCharacter = (char)value;

    recoveredMessage[*messageIndex] = recoveredCharacter;
    (*messageIndex) = (*messageIndex) + 1;
    recoveredMessage[*messageIndex] = '\0';

    printf(" [APP LAYER]   Recovered character: '%c'\n", recoveredCharacter);
    return 1;
}

/* ---------------- Main Function ---------------- */

int main(void) {
    FILE *channel;
    char pdu[BUFFER_SIZE];
    char recoveredMessage[MESSAGE_SIZE];
    int messageIndex = 0;
    int frameNumber = 0;

    channel = fopen(CHANNEL_FILE, "r");
    if (channel == NULL) {
        printf("Cannot open channel file. Run sender first.\n");
        return 1;
    }

    recoveredMessage[0] = '\0';

    printf("========================================================\n");
    printf("     OSI RECEIVER - BYTE COUNT PROTOCOL + CHECKSUM      \n");
    printf("========================================================\n");

    while (fscanf(channel, "%s", pdu) == 1) {
        frameNumber = frameNumber + 1;
        printf("\n--- Decapsulating frame %d ---\n", frameNumber);

        if (!dataLinkReceiver(pdu)) {
            printf(" [RECEIVER]    Frame dropped.\n");
            continue;
        }
        if (!networkReceiver(pdu)) {
            printf(" [RECEIVER]    Frame dropped.\n");
            continue;
        }
        if (!transportReceiver(pdu)) {
            printf(" [RECEIVER]    Frame dropped.\n");
            continue;
        }
        if (!applicationReceiver(pdu, recoveredMessage, &messageIndex)) {
            printf(" [RECEIVER]    Frame dropped.\n");
            continue;
        }

        printf(" [RECEIVER]    Frame accepted.\n");
    }

    fclose(channel);
    printf("\n========================================================\n");
    printf(" Final reassembled message: \"%s\"\n", recoveredMessage);
    printf("========================================================\n");
    return 0;
}
