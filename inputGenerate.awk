#!/usr/bin/awk -f

BEGIN{
    k = 0;
    printf("Generating input files......\n");
}

/^knapPI/{
    if(k < count){
        k++;
        str = sprintf("%s/inputFile%d.csv", testset, k);
        printf("%s\n",str);
        getline line;
        tmp = split(line, a);
        n = a[2];
        getline line;
        tmp = split(line, a);
        c = a[2];
        printf("%d,%d\n", n, c) > str; 

        getline line;
        getline line;

        for(i = 1; i <= n; i++){
            getline line;
            tmp = split(line, a, ",");
            p = a[2];
            w = a[3];
            printf("%d,%d\n", p, w) >> str;
        }
    }
}

END{
    printf("Finished!\n");
    printf("The count of files generated is: %d\n", k);  
}
