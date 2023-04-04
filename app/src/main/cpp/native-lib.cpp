#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/stat.h>

const char* TAG = "MyNativeCode";
bool IS_SERVER_RUNNING = true;
const char* SOCKET_NAME = "mysocket";
const char* FILE_SOCKET_NAME_PATH = "/data/user/0/com.example.socket/files/mydir/";


void my_native_function() {
    const char* message = "Hello from my native function!";
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", message);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_socket_MainActivity_stringFromJNI(JNIEnv* env, jobject /* this */) {

    my_native_function();

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_startServer(JNIEnv *env, jobject /* this */) {

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error creating socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "server_socket=: %i", server_socket);

    // create a Unix domain socket in the abstract namespace
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    // set the first byte to a null character to create a socket in the abstract namespace
    server_address.sun_path[0] = '\0';
    // set the socket name using the remaining bytes of sun_path
    strcpy(&server_address.sun_path[1], SOCKET_NAME);

//    char address_str[108]; // assuming a maximum address length of 108 bytes
//    snprintf(address_str, sizeof(address_str), "%s", &server_address.sun_path[1]); // copy the address string without the null character
//    __android_log_print(ANDROID_LOG_INFO, tag, "Binding to socket address: %s", address_str); // print the address string

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error binding socket: %s", strerror(errno));
        return 1;
    }

    if (listen(server_socket, 1) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error listen socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "Server started listening...");

    int client_socket = accept(server_socket, nullptr, nullptr);

    if (client_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error accept socket: %s", strerror(errno));
        return 1;
    }

    std::cout << "Client connected." << std::endl;
    __android_log_print(ANDROID_LOG_INFO, TAG, "Client connected.");

    char buf[256];
    int n = read(client_socket, buf, sizeof(buf));

    if (n == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error read socket: %s", strerror(errno));
        return 1;
    }

    buf[n] = '\0'; // add null terminator to the received message
    std::cout << "Received message from client: " << buf << std::endl;
    __android_log_print(ANDROID_LOG_INFO, TAG, "Received message from client: %s", buf);

    const char* response = "Hello, client!";
    if (write(client_socket, response, strlen(response)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error sending message: %s", strerror(errno));
        close(client_socket);
        close(server_socket);
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "Sent message to client: %s", response);

    close(client_socket);
    close(server_socket);

    return 0;
}

/*
 Example:
 ❯ adb shell "cat /proc/net/unix" | grep -a "mysocket"
   0000000000000000: 00000002 00000000 00010000 0001 01 21495382 @mysocket@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 */
// why you get to many '@@@@' in the socket name
// that Unix domain sockets have a fixed length of 108 bytes, and the remaining bytes are filled with null characters.
// the @ character represents the null character.
// ^---- this because
//         create a name with this 'strncpy(&server_address.sun_path[1], SOCKET_NAME, sizeof(server_address.sun_path) - 2);'
//         but if you use this 'strcpy(&server_address.sun_path[1], SOCKET_NAME);' you will not get '@' in the end


extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_startServerWithAbstract(JNIEnv *env, jobject /* this */) {
    IS_SERVER_RUNNING = true;

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error creating socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "server_socket=: %i", server_socket);

    // create a Unix domain socket in the abstract namespace
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    // set the first byte to a null character to create a socket in the abstract namespace
    server_address.sun_path[0] = '\0';
    // set the socket name using the remaining bytes of sun_path
    strcpy(&server_address.sun_path[1], SOCKET_NAME);  // this line will create abstract socket name without '@' append in the end
    // strncpy(&server_address.sun_path[1], SOCKET_NAME, sizeof(server_address.sun_path) - 2);

    // unlink("\0mysocket");

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error binding socket: %s", strerror(errno));
        return 1;
    }

    if (listen(server_socket, 1) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error listen socket: %s", strerror(errno));
        return 1;
    }

    // Print the socket address after the socket is bound and listening
    socklen_t address_length = sizeof(server_address);
    if (getsockname(server_socket, (struct sockaddr *)&server_address, &address_length) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error getting socket address: %s", strerror(errno));
        return 1;
    }
    char address_str[108]; // assuming a maximum address length of 108 bytes
    snprintf(address_str, sizeof(address_str), "%s", &server_address.sun_path[1]); // copy the address string without the null character
    __android_log_print(ANDROID_LOG_INFO, TAG, "Server started listening on address: %s", address_str);

    while (IS_SERVER_RUNNING) {
        int client_socket = accept(server_socket, nullptr, nullptr);

        if (client_socket == -1) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Error accept socket: %s", strerror(errno));
            continue;
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "Client connected.");

        char buf[256];
        int n = read(client_socket, buf, sizeof(buf));

        if (n == -1) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Error read socket: %s", strerror(errno));
            close(client_socket);
            continue;
        }

        buf[n] = '\0'; // add null terminator to the received message
        __android_log_print(ANDROID_LOG_INFO, TAG, "Received message from client: %s", buf);

        const char* response = "Hello, client!";
        if (write(client_socket, response, strlen(response)) == -1) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Error sending message: %s", strerror(errno));
            close(client_socket);
            continue;
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "Sent message to client: %s", response);

        close(client_socket);
    }

    close(server_socket);

    return 0;
}

/**
 0000000000000000: 00000002 00000000 00010000 0001 01 48600 @mysocket
❯ adb shell "cat /proc/net/unix" | grep -a "mysocket"
0000000000000000: 00000002 00000000 00010000 0001 01 82113 /data/user/0/com.example.socket/files/mydir/mysocket
 */

extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_startServerWithFileSystem(JNIEnv *env, jobject /* this */) {
    IS_SERVER_RUNNING = true;

    int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error creating socket: %s", strerror(errno));
        return 1;
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "server_socket=: %i", server_socket);

    // create the /tmp directory if it does not exist
//    if (mkdir("/tmp", 0777) == -1 && errno != EEXIST) {
//        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error creating directory: %s", strerror(errno));
//        return 1;
//    }

    // create a Unix domain socket in the filesystem namespace
    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    std::string socket_name = std::string(FILE_SOCKET_NAME_PATH) + std::string(SOCKET_NAME);
    strncpy(server_address.sun_path, socket_name.c_str(), sizeof(server_address.sun_path) - 1);
    //unlink(server_address.sun_path);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error binding socket: %s", strerror(errno));
        return 1;
    }

    if (listen(server_socket, 1) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error listen socket: %s", strerror(errno));
        return 1;
    }

    // Print the socket address after the socket is bound and listening
    socklen_t address_length = sizeof(server_address);
    if (getsockname(server_socket, (struct sockaddr *)&server_address, &address_length) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error getting socket address: %s", strerror(errno));
        return 1;
    }
    char address_str[108]; // assuming a maximum address length of 108 bytes
    snprintf(address_str, sizeof(address_str), "%s", &server_address.sun_path[1]); // copy the address string without the null character
    __android_log_print(ANDROID_LOG_INFO, TAG, "Server started listening on address: %s", address_str);

    while (IS_SERVER_RUNNING) {
        int client_socket = accept(server_socket, nullptr, nullptr);

        if (client_socket == -1) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Error accept socket: %s", strerror(errno));
            continue;
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "Client connected.");

        char buf[256];
        int n = read(client_socket, buf, sizeof(buf));

        if (n == -1) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Error read socket: %s", strerror(errno));
            close(client_socket);
            continue;
        }

        buf[n] = '\0'; // add null terminator to the received message
        __android_log_print(ANDROID_LOG_INFO, TAG, "Received message from client: %s", buf);

        const char* response = "Hello, client!";
        if (write(client_socket, response, strlen(response)) == -1) {
            __android_log_print(ANDROID_LOG_ERROR, TAG, "Error sending message: %s", strerror(errno));
            close(client_socket);
            continue;
        }

        __android_log_print(ANDROID_LOG_INFO, TAG, "Sent message to client: %s", response);

        close(client_socket);
    }

    close(server_socket);

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_stopServer(JNIEnv *env, jobject /* this */) {
    IS_SERVER_RUNNING = false;
    return 0;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_socket_MainActivity_talkToAbstractSocketServer(JNIEnv *env, jobject /* this */) {
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error creating socket: %s", strerror(errno));
        return env->NewStringUTF("");
    }

    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    server_address.sun_path[0] = '\0';
    strcpy(&server_address.sun_path[1], SOCKET_NAME); // this line will create abstract socket name without '@' append in the end
    // strncpy(&server_address.sun_path[1], SOCKET_NAME, sizeof(server_address.sun_path) - 2);


    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error connecting to socket: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "Connected to server.");

    const char* message = "Hello, server!";
    if (write(client_socket, message, strlen(message)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error sending message: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "Sent message to server: %s", message);

    char buf[256];
    int n = read(client_socket, buf, sizeof(buf));

    if (n == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error receiving message: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    buf[n] = '\0'; // add null terminator to the received message
    __android_log_print(ANDROID_LOG_INFO, TAG, "Received message from server: %s", buf);

    close(client_socket);

    return env->NewStringUTF(buf);
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_example_socket_MainActivity_talkToFileSystemSocketServer(JNIEnv *env, jobject /* this */) {
    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error creating socket: %s", strerror(errno));
        return env->NewStringUTF("");
    }

    struct sockaddr_un server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    std::string socket_name = std::string(FILE_SOCKET_NAME_PATH) + std::string(SOCKET_NAME);
    strncpy(server_address.sun_path, socket_name.c_str(), sizeof(server_address.sun_path) - 1);

    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error connecting to socket: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "Connected to server.");

    const char* message = "Hello, server!";
    if (write(client_socket, message, strlen(message)) == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error sending message: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    __android_log_print(ANDROID_LOG_INFO, TAG, "Sent message to server: %s", message);

    char buf[256];
    int n = read(client_socket, buf, sizeof(buf));

    if (n == -1) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Error receiving message: %s", strerror(errno));
        close(client_socket);
        return env->NewStringUTF("");
    }

    buf[n] = '\0'; // add null terminator to the received message
    __android_log_print(ANDROID_LOG_INFO, TAG, "Received message from server: %s", buf);

    close(client_socket);

    return env->NewStringUTF(buf);
}
