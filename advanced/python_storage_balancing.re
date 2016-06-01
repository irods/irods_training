findRescType(*INST_NAME, *OUT) {
    foreach ( *ROW in SELECT RESC_TYPE_NAME WHERE RESC_NAME = '*INST_NAME' ) {
        *OUT = *ROW.RESC_TYPE_NAME;
    }
}

findInstId(*INST_NAME, *OUT) {
    foreach ( *ROW in SELECT RESC_ID WHERE RESC_NAME = '*INST_NAME' ) {
        *OUT = *ROW.RESC_ID;
    }
}

findBytesUsed(*INST_ID, *OUT) {
    foreach ( *ROW1 in SELECT RESC_NAME WHERE RESC_PARENT = '*INST_ID' ) {
        *STORAGE_RESC = *ROW1.RESC_NAME;
        *TEMP = 0
        foreach ( *ROW2 in SELECT sum(DATA_SIZE) WHERE RESC_NAME = '*STORAGE_RESC' ) {
            *TEMP = *TEMP + int(*ROW2.DATA_SIZE)
        }
        *OUT = "*TEMP"
    }
}

findContextString(*INST_NAME, *OUT) {
    foreach ( *ROW in SELECT RESC_CONTEXT WHERE RESC_NAME = '*INST_NAME' ) {
        *OUT = *ROW.RESC_CONTEXT;
    }
}

