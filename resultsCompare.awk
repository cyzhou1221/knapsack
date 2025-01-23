#!/usr/bin/awk -f

BEGIN{
    i = 0;  # for seq
    j = 0;  # for par 
    threads_string = "";
}

/Your job looked like:/{
    k = 1;
    while(k <= 4){
        getline line;
        k++;
    }
    tmp = split(line, a);
    
    if(a[1] ~ /s$/){
        i++;
        isSEQ = 1;
    }
    else{
        j++;
        isSEQ = 0;
    }

    getline line;
    getline line;
    getline line;

    if(line ~ /^Successfully completed./){
        while(!(line ~ /^Read problem from:/)){
            getline line;
        }
        getline line;
        if(isSEQ == 1){
            sSuccess[i] = 1;
            tmp = split(line, a);
            sRealTime[i] = a[8];
            sCPUTime[i] = a[13];
            sNodeCount[i] = a[18]; 
        }
        else{
            pSuccess[j] = 1;
            tmp = split(line, a);
            pRealTime[j] = a[8];
            pCPUTime[j] = a[13];
            pNodeCount[j] = a[18];
        }
    }
    else{
        if(isSEQ == 1){
            sSuccess[i] = 0;
            sRealTime[i] = "timeout";
            sCPUTime[i] = "timeout";
            sNodeCount[i] = "--";  
        }
        else{
            pSuccess[j] = 0;
            pRealTime[j] = "timeout";
            pCPUTime[j] = "timeout";
            pNodeCount[j] = "--";
        }
    }
}

/^Max threads count is: /{
    if(!(threads_string ~ /^Max threads count is: /)){
        threads_string = $0;
        threads_count = $5;
    }
}

END{ 
    printf("\n");
    printf("+--------------+----------------+------------+----------------+------------+--------+--------+\n");
    printf("| Instance     | sRealTime(sec) | sNodeCount | pRealTime(sec) | pNodeCount | tRatio | nRatio |\n");
    printf("+--------------+----------------+------------+----------------+------------+--------+--------+\n");  
    str = sprintf("resultTable_%dthread.tex",  threads_count);
    printf("%s\n\n", str) > str;
    
    sSuccessNum = 0;
    pSuccessNum = 0;
    bothSuccess = 0;
    sTimeSum = 0;
    pTimeSum = 0;
    sNodeSum = 0;
    pNodeSum = 0;
    count = i;
    for(i = 1; i <= count; i++){
        # statistic
        sSuccessNum += sSuccess[i];
        pSuccessNum += pSuccess[i];
        printf("| %-12s", sprintf("inputFile%d", i));
        printf("%-12s", sprintf("inputFile%d", i)) >> str;
        if(sSuccess[i] == 1){
            printf(" |%15.2f |%11d", sRealTime[i], sNodeCount[i]);
            printf(" &%15.2f &%11d", sRealTime[i], sNodeCount[i]) >> str;
        }
        else{
            printf(" |%15s |%11s", sRealTime[i], sNodeCount[i]);
            printf(" &%15s &%11s", sRealTime[i], sNodeCount[i]) >> str;
        }
        if(pSuccess[i] == 1){
            printf(" |%15.2f |%11d", pRealTime[i], pNodeCount[i]);
            printf(" &%15.2f &%11d", pRealTime[i], pNodeCount[i]) >> str;
        }
        else{
            printf(" |%15s |%11s", pRealTime[i], pNodeCount[i]);
            printf(" &%15s &%11s", pRealTime[i], pNodeCount[i]) >> str;
        }
        if(sSuccess[i] == 1 && pSuccess[i] == 1){
            bothSuccess++;
            if(pRealTime[i] <= 10e-6){
                printf(" |%7s", "--");
                printf(" &%7s", "--") >> str;
            }
            else{
                printf(" |%7.2f", sRealTime[i]/pRealTime[i]);
                printf(" &%7.2f", sRealTime[i]/pRealTime[i]) >> str;
            }
            printf(" |%7.2f |\n", sNodeCount[i]/pNodeCount[i]);
            printf(" &%7.2f  \\\\\n", sNodeCount[i]/pNodeCount[i]) >> str;

            # statistic
            sTimeSum += sRealTime[i];
            pTimeSum += pRealTime[i];
            sNodeSum += sNodeCount[i];
            pNodeSum += pNodeCount[i];
        }
        else{
            printf(" |%7s |%7s |\n", "--", "--");
            printf(" &%7s &%7s  \\\\\n", "--", "--") >> str;
        }
    }
    printf("+--------------+----------------+------------+----------------+------------+--------+--------+\n");
    
    if(!(bothSuccess == 0)){
        sTimeAver = sTimeSum/bothSuccess;
        pTimeAver = pTimeSum/bothSuccess;
        sNodeAver = sNodeSum/bothSuccess;
        pNodeAver = pNodeSum/bothSuccess;
    }
    if(pTimeSum <= 10e-6){
        timeRatio = "--";
    }
    else{
        timeRatio = sTimeSum/pTimeSum;
    }
    if(pNodeSum == 0){
        nodeRatio = "--";
    }
    else{
        nodeRatio = sNodeSum/pNodeSum;
    }

    printf("\n%s\n", threads_string);
    printf("Count of instances is: %d\n", count);
    printf("Count of instances solved by sequential algorithm is: %d, by parallel algorithm is: %d\n", sSuccessNum, pSuccessNum);
    printf("Count of instances that both successed is: %d\n", bothSuccess);

    if(!(bothSuccess == 0)){
        printf("Average time of Node_Processing by sequential algorithm is: %10.2f, by parallel algorithm is: %10.2f\n", sTimeAver, pTimeAver);
        printf("Average count of nodes by sequential algorithm is: %10.2f, by parallel algorithm is: %10.2f\n", sNodeAver, pNodeAver);    
        printf("Total ratio of time is: ");
        if(pTimeSum <= 10e-6){
            printf("%10s\n", timeRatio);
        }
        else{
            printf("%10.2f\n", timeRatio);
        }
        printf("Total ratio of node is: ");
        if(pNodeSum == 0){
            printf("%10s\n", nodeRatio);
        }
        else{
            printf("%10.2f\n", nodeRatio);
        }
    }
}
