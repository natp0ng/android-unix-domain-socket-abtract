#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

const char* tag = "MyNativeCode";
bool server_running = true;

void my_native_function() {
    const char* message = "Hello from my native function!";
    __android_log_print(ANDROID_LOG_INFO, tag, "%s", message);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_socket_MainActivity_stringFromJNI(JNIEnv* env, jobject /* this */) {

    my_native_function();

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

/*
 Example:
 ❯ adb shell "cat /proc/net/unix" | grep -a "mysocket"
   0000000000000000: 00000002 00000000 00010000 0001 01 21495382 @mysocket@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 */

extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_startServer(JNIEnv *env, jobject /* this */) {

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error creating socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "server_socket=: %i", server_socket);

    // create a Unix domain socket in the abstract namespace
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    // set the first byte to a null character to create a socket in the abstract namespace
    server_address.sun_path[0] = '\0';
    // set the socket name using the remaining bytes of sun_path
    strcpy(&server_address.sun_path[1], "mysocket");

//    char address_str[108]; // assuming a maximum address length of 108 bytes
//    snprintf(address_str, sizeof(address_str), "%s", &server_address.sun_path[1]); // copy the address string without the null character
//    __android_log_print(ANDROID_LOG_INFO, tag, "Binding to socket address: %s", address_str); // print the address string

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error binding socket: %s", strerror(errno));
        return 1;
    }

    if (listen(server_socket, 1) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error listen socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "Server started listening...");

    int client_socket = accept(server_socket, nullptr, nullptr);

    if (client_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error accept socket: %s", strerror(errno));
        return 1;
    }

    std::cout << "Client connected." << std::endl;
    __android_log_print(ANDROID_LOG_INFO, tag, "Client connected.");

    char buf[256];
    int n = read(client_socket, buf, sizeof(buf));

    if (n == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error read socket: %s", strerror(errno));
        return 1;
    }

    buf[n] = '\0'; // add null terminator to the received message
    std::cout << "Received message from client: " << buf << std::endl;
    __android_log_print(ANDROID_LOG_INFO, tag, "Received message from client: %s", buf);

    const char* response = "Hello, client!";
    if (write(client_socket, response, strlen(response)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error sending message: %s", strerror(errno));
        close(client_socket);
        close(server_socket);
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "Sent message to client: %s", response);

    close(client_socket);
    close(server_socket);

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_startServer1(JNIEnv *env, jobject /* this */) {
    server_running = true;

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error creating socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "server_socket=: %i", server_socket);

    // create a Unix domain socket in the abstract namespace
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    // set the first byte to a null character to create a socket in the abstract namespace
    server_address.sun_path[0] = '\0';
    // set the socket name using the remaining bytes of sun_path
    strcpy(&server_address.sun_path[1], "mysocket");

//    unlink("\0mysocket");

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error binding socket: %s", strerror(errno));
        return 1;
    }

    if (listen(server_socket, 1) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error listen socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "Server started listening...");

    while (server_running) {
        int client_socket = accept(server_socket, nullptr, nullptr);

        if (client_socket == -1) {
            __android_log_print(ANDROID_LOG_ERROR, tag, "Error accept socket: %s", strerror(errno));
            continue;
        }

        std::cout << "Client connected." << std::endl;
        __android_log_print(ANDROID_LOG_INFO, tag, "Client connected.");

        char buf[256];
        int n = read(client_socket, buf, sizeof(buf));

        if (n == -1) {
            __android_log_print(ANDROID_LOG_ERROR, tag, "Error read socket: %s", strerror(errno));
            close(client_socket);
            continue;
        }

        buf[n] = '\0'; // add null terminator to the received message
        std::cout << "Received message from client: " << buf << std::endl;
        __android_log_print(ANDROID_LOG_INFO, tag, "Received message from client: %s", buf);

        const char* response = "Hello, client!";
        if (write(client_socket, response, strlen(response)) == -1) {
            __android_log_print(ANDROID_LOG_ERROR, tag, "Error sending message: %s", strerror(errno));
            close(client_socket);
            continue;
        }

        __android_log_print(ANDROID_LOG_INFO, tag, "Sent message to client: %s", response);

        close(client_socket);
    }

    close(server_socket);

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_stopServer(JNIEnv *env, jobject /* this */) {
    server_running = false;
    return 0;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_socket_MainActivity_talkToServer(JNIEnv *env, jobject /* this */) {
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error creating socket: %s", strerror(errno));
        return env->NewStringUTF("");
    }

    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    server_address.sun_path[0] = '\0';
    strcpy(&server_address.sun_path[1], "mysocket");

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error connecting to socket: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "Connected to server.");

    const char* message = "Hello, server!";
    if (write(client_socket, message, strlen(message)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error sending message: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "Sent message to server: %s", message);

    char buf[256];
    int n = read(client_socket, buf, sizeof(buf));

    if (n == -1) {
        __android_log_print(ANDROID_LOG_ERROR, tag, "Error receiving message: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    buf[n] = '\0'; // add null terminator to the received message
    __android_log_print(ANDROID_LOG_INFO, tag, "Received message from server: %s", buf);

    close(client_socket);

    return env->NewStringUTF(buf);
}
