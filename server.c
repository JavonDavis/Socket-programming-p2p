#include <stdio.h>
#include <string.h>   //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
  
#define PORT 60000
 
int main(int argc , char *argv[])
{
    int main_socket , addrlen , new_socket , client_socket[10] , max_clients = 10 , activity, i , num_bytes , sd;
    int max =10,send_len;
    int maxfds;
    int count = 0;
    int ips[30];
    struct sockaddr_in address,recv_addr;
    char names[3][14];
    char name[80],response[80];
    char online[5][2];
    int listening[5];
    char reply[30];
    char buffer[1025],buff[1024];  //data buffer of 1K
      

    int working_group[10], fun_group[10];
    int workingIndex = 0, funIndex = 0;
    //set of socket descriptors
    fd_set readfds, tempfds;
    FD_ZERO(&readfds);

    FILE *fp = fopen("busy.txt","w");
    fclose(fp);
      
    //a message
    char message[80];
    for(i = 0; i<max_clients; i++)
    {
        client_socket[i] = 0;
    }
    main_socket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
    if(main_socket == 0)
    {
        perror("socket creation failed\n");
        exit(0);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to the port
    i = bind(main_socket, (struct sockaddr *) &address,sizeof(address));
    if(i<0)
    {
        perror("bind failed");
        exit(0);
    }

    printf("Listener on port %d \n", PORT);
     
    FD_SET(main_socket, &readfds);
    addrlen = sizeof(address);
    printf("Waiting for connections...\n");

     //try to specify maximum of 3 pending connections for the master socket
    i = listen(main_socket, 3) ;
    if (i < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        
       FD_ZERO(&readfds);
       FD_SET(main_socket,&readfds);
       
        maxfds = main_socket;
        for (i = 0; i < max_clients; i++){

            sd = client_socket[i];

            if(sd> 0)
            {
                FD_SET(sd,&readfds);
            }

            if (client_socket[i] > maxfds){
                maxfds =sd;
            }
        }

        activity = select(maxfds + 1, &readfds, NULL, NULL, NULL);

        if(activity<0)
        {
            perror("activity");
        }

        if (FD_ISSET(main_socket, &readfds)) {
            //activity on the main
            //accept connection from an incoming client
            sd = accept(main_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (sd < 0)
            {
                perror("accept failed");
                return 1;
            }
            printf("Connection accepted from %d\n",sd);

            memset(name,'\0',2000);
            //Receive name from client
            i = recv(sd , name , 80 , 0);
            printf("name = %s\n",name);
            strcpy(names[count],name);
            //client_socket[count] = sd;
            ips[count] = address.sin_addr.s_addr;
            client_socket[count] = sd;
            count+=1;  

            FILE *f = fopen("online.txt","w");
            fprintf(f,"%d\n",count);
            
            for(i=0;i<count;i++)
            {   
                printf("%s\n",names[i]);
                fprintf(f,"%s\n",names[i]);    //username
                fprintf(f,"%d\n",client_socket[i]);  //socket descriptor
                fprintf(f,"%d\n",ips[i]);    //ip
            }
            fclose(f);

            for(i=0;i<max_clients;i++)
            {
                if(client_socket[i] == 0)
                {
                    client_socket[i] = sd;
                    break;
                }
            }

            write(sd,"Name saved\0",11);

        }

        // if(!fork())
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)){
                
                char current[80],accept[1];
                char online[5][10];
                int busy;
                int oip[10],sock_desc_client[10];
                //char message[80];
                memset(response,'\0',2000);
               // printf("forked process\n");
                int size;
                char user[80];

                // while((size = recv(sd,response,2000,0)) >0)
                // {
                // while (1){
                size = recv(sd, response, 2000, 0);
                response[strlen(response)] = 0;
                    // response[strlen(response)-1] = '\0';

                if(size == 0)
                {
                    printf("Client %d disconnected\n",sd);

                    close(sd);
                    FD_CLR(sd,&readfds);
                    //fflush(stdout);
                    break;
                }
                    if(strcmp(response,"\\c")==0)
                    {
                        //printf("Here\n");
                        //Begin chat process 
                        memset(response,'\0',2000);
                        recv(sd,response,2000,0);//stage 1 get name of other client

                        printf("Request to connect to %s\n",response);

                        int k = 0;
                        //char list[100];
                        //char socket[80];
                        //char ip[80];
                        int new_sock,new_ip,found =0;
                        FILE *f = fopen("online.txt","r");
                        fscanf(f,"%d",&count);

                        for(i=0;i<count;i++)
                        {
                            fscanf(f,"%s",user);     //username
                            if(!strcmp(user,response))
                            {
                                found = 1;
                                k=i;
                            }
                                
                                
                            fscanf(f,"%d",&new_sock);    //socket desc
                            
                                
                            fscanf(f,"%d",&new_ip);   //ip
                        }
                        fclose(f);


                        // for(k=0;k<max;k++)
                        // {
                        //     if(client_socket[k]!=0)
                        //     {
                        //         if(strcmp(names[k],response) ==0)
                        //         {
                        //             new_sock = client_socket[k];
                        //             new_ip = ips[k];
                        //             found =1;
                        //         }
                        //     }
                            

                        // }

                        if(!found)
                        {
                            printf("Name not found.\n");
                            memset(reply,'\0',1);
                            strcpy(reply,"n");
                            write(sd,reply,1);
                        }
                        else
                        {
                            found =0;
                            printf("found user %s\n",response);
                            memset(reply,'\0',1); //TODO - investigate 
                            strcpy(reply,"b");

                            printf("Checking if user is busy\n");

                            write(sd,"Checking if user is busy\n\0",26);

                            // printf("Sock:%d, msg:%s\n", new_sock,reply);
                            // write((int) new_sock,reply,1);

                            FILE *f = fopen("online.txt","r");
                            fscanf(f,"%d\n",&count);
            
                            for(i=0;i<=count;i++)
                            {   
                                fscanf(f,"%s",online[i]);
                                fscanf(f,"%d",&sock_desc_client[i]);
                                fscanf(f,"%d",&oip[i]);
                            }
                            fclose(f);

                            printf("%d\n", sock_desc_client[k]);

                            // write(sock_desc_client[k],reply,strlen(reply));

                            // if(write<0)
                            // {
                            //     perror("write");
                            // }

                            int tmp;

                            busy =0;
                            FILE *f1 = fopen("busy.txt","r");


                            //if(f1!=NULL)
                            for(i=0;i<=max_clients;i++)
                            {   
                                if(fscanf(f1,"%d",&tmp)>0)
                                    if(tmp==sock_desc_client[k])
                                    {
                                        busy = 1;
                                    }
                            }
                            fclose(f1);

                            printf("here\n");
                            //recv(sock_desc_client[k],busy,80,0);
                            printf("busy?%d\n", busy);
                            if(busy)
                            {
                                write(sd,"0",42);
                            }
                            else //setup p2p
                            {
                                //write(sd,"Client not busy attempting to connect.\0",39);
                                int p;
                                for(p=0;p<max_clients;p++)
                                {

                                    if(sock_desc_client[p] == sd)
                                    {   
                                        
                                        strcpy(current,online[p]);
                                        break;
                                    }
                                    

                                }
                                
                                //memset(reply,'\0',2000);
                                strcpy(reply,current);
                                
                                strcat(reply," would like to chat with you. Accept?(y/n)\n");

                                printf("%s\n", reply);

                                if(write(sock_desc_client[k],reply,strlen(reply))<0)
                                {
                                    perror("write");
                                    return 1;
                                }

                                memset(message,'\0',2000);
                                recv(sock_desc_client[k],message,80,0);
                                printf("%s\n", message);
                                if(strcmp(message,"y")==0)
                                {
                                    printf("Messagei:%s\n", message);
                                    memset(reply,'\0',2000);
                                    int new_port;
                                    char temp[80];
                                    new_port = PORT + (k+1)*2;

                                    sprintf(temp,"%d",oip[k]);
                                    strcpy(reply,"@");
                                    strcat(reply,temp);
                                    strcat(reply,",");

                                    memset(temp,'\0',80);
                                    sprintf(temp,"%d",new_port);

                                    write(sd,temp,strlen(temp));

                                    strcat(reply,temp);

                                    printf("Message:%s\n", reply);

                                    sleep(1);

                                    write(sock_desc_client[k],reply,strlen(reply));

                                    FILE *f = fopen("busy.txt","a");
                                    fprintf(f,"%d\n",sock_desc_client[k]);
                                    fprintf(f,"%d\n",sd);
                                    fclose(f);
                                    // close(sd);
                                    // sd = -1;
                                    // close(sock_desc_client[k]);
                                    // sock_desc_client[k] = -1;

                                    // FD_CLR(sd,&readfds);
                                    // FD_CLR(sock_desc_client[k],&readfds);
                                    // printf("Convo initiated\n");
                                }
                            }
                        }
                    }
                    else if (strcmp(response, "\\w") == 0)
                    {
                        char * confirmation = "You have been added to the working gruop.";
                        // Check if sd is not in working group array..
                        working_group[workingIndex] = sd;
                        workingIndex++;
                        //printf("%d has been added to the working group\n", sd);
                        write(sd, confirmation, strlen(confirmation));
                        //break;
                    }

                    else if (strcmp(response, "\\f") == 0)
                    {
                        char * confirmation = "You have been added to the fun group.";
                        // Check if sd is not in working group array..
                        fun_group[funIndex] = sd;
                        funIndex++;
                        //printf("%d has been added to the fun group\n", sd);
                        write(sd, confirmation, strlen(confirmation));
                        //break;
                    }

                    else if (strcmp(response, "\\sw") == 0)
                    {
                        int found = 0;
                        char * error = "You aren't in the working group.";
                        // Get message..
                        memset(response,'\0',2000); //clear response variable
                        recv(sd,response,2000,0);//stage 1 get name of other client

                        printf("working index:%d\n", workingIndex);
                        for (i = 0; i < max_clients; i++)
                        {
                            int wsd = working_group[i];
                            if ( sd == wsd )
                                found = 1;
                        }
                        if (found){
                            // loop over working group and send to all..
                            for (i = 0; i < max_clients; i++)
                            {
                                int wsd = working_group[i]; 
                                if (wsd > 0 && wsd != sd)
                                    write(wsd, response, strlen(response));
                            }
                        } else 
                            write(sd, error, strlen(error));
                    }

                    else if (strcmp(response, "\\sf") == 0)
                    {
                        int found = 0;

                        char * error = "You aren't in the fun group.";
                        // Get message..
                        memset(response,'\0',2000); //clear response variable
                        recv(sd,response,2000,0);//stage 1 get name of other client
                        // loop over working group and send to all..

                        for (i = 0; i < max_clients; i++)
                        {
                            int fsd = fun_group[i];
                            if ( sd == fsd )
                                found = 1;
                        }
                        if (found){
                            for (i = 0; i < max_clients; i++)
                            {
                                int fsd = fun_group[i]; 
                                if (fsd > 0 && fsd != sd)
                                    write(fsd, response, strlen(response));
                                }
                        } else 
                            write(sd, error, strlen(error));
                    }

                    else if(strcmp(response,"\\l") == 0)
                    {
                        int k = 0;
                        char list[100];
                        char socket[80];
                        char ip[80];
                        char tuser[80];
                        char temp[80];
                        int scl,var;

                        int count;
                        char user[80];
                        strcat(list,"Available USERS : \n");
                        FILE *f = fopen("online.txt","r");
                        fscanf(f,"%d",&count);
                        
                        for(i=0;i<count;i++)
                        {
                            fscanf(f,"%s",tuser);
                            fscanf(f,"%d",&scl);
                            fscanf(f,"%d",&var);
                            
                            sprintf(temp,"User:%s\n",tuser);
                            strcat(list,temp);
                        }
                        // fclose(f); 
                        // for(k=0;k<max;k++)
                        // {
                        //     if(client_socket[k]!=0)
                        //     {
                        //         printf("%d\n", k);
                        //        strcat(list,names[k]);
                        //         strcat(list,"\t");
                        //         sprintf(socket,"%d",client_socket[k]);
                        //         sprintf(ip,"%d",ips[k]);
                        //         strcat(list,socket);
                        //         strcat(list,"\t");
                        //         strcat(list,ip);
                        //         strcat(list,"\n"); 
                        //     }
                            

                        // }
                        write(sd,list,strlen(list));
                        memset(list,'\0',sizeof(list));
                    }
                    else if(strcmp(response,"\\x") ==0)
                    {
                        FILE *f1 = fopen("busy.txt","r");
                        FILE *f2 = fopen("busy.txt","w");
                        int tmp;
                            //if(f1!=NULL)
                        for(i=0;i<=max_clients;i++)
                        {   
                            if(fscanf(f1,"%d",&tmp)>0)
                                if(tmp!=sd)
                                {
                                    fprintf(f2, "%d\n", tmp);
                                }
                        }
                        fclose(f1);
                        fclose(f2);
                    }
                // }
            }
           
        }      
    }

    // close all sockets in 

    remove("busy.txt");
}