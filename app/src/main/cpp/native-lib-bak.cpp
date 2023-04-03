#include <jni.h>
#include <string>
#include <android/log.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

const char* tag = "MyNativeCode";

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

//extern "C" JNIEXPORT jstring JNICALL
//Java_com_example_socket_MainActivity_stopServer(JNIEnv *env, jobject /* this */) {
//    // Close the server socket
//    close(server_socket_fd);
//    return env->NewStringUTF("Server stopped!");
//}

extern "C" JNIEXPORT jint JNICALL
Java_com_example_socket_MainActivity_startServer(JNIEnv *env, jobject /* this */) {
    struct sockaddr_un server_address;
    int server_socket_fd = -1;

    // Create a new server socket
    server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        std::cerr << "Error creating server socket: " << strerror(errno) << std::endl;
        return -1;
    }

    // Set the socket address to the abstract socket address
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    std::strcpy(server_address.sun_path, "\0mysocket");
    server_address.sun_path[0] = '\0';

    __android_log_print(ANDROID_LOG_INFO, tag, "%s", "sever debug 0");

    // Bind the socket to the abstract socket address
    if (bind(server_socket_fd, (struct sockaddr *)&server_address, sizeof(sa_family_t) + strlen(server_address.sun_path)) == -1) {
        std::cerr << "Error binding server socket: " << strerror(errno) << std::endl;
        close(server_socket_fd);
        return -1;
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "%s", "sever debug 1");

    // Listen for incoming connections
    if (listen(server_socket_fd, 5) == -1) {
        std::cerr << "Error listening on server socket: " << strerror(errno) << std::endl;
        close(server_socket_fd);
        return -1;
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "%s", "sever debug 3");

    // Wait for incoming connections and handle them
    while (true) {
        __android_log_print(ANDROID_LOG_INFO, tag, "%s", "sever debug 4");
        struct sockaddr_un client_address;
        socklen_t client_address_length = sizeof(client_address);
        int client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket_fd == -1) {
            std::cerr << "Error accepting client connection: " << strerror(errno) << std::endl;
            __android_log_print(ANDROID_LOG_INFO, tag, "Error accepting client connection %s", strerror(errno));
            continue;
        }

        // Read the command from the client
        char buffer[256] = {0};
        ssize_t bytes_received = recv(client_socket_fd, buffer, sizeof(buffer), 0);
        if (bytes_received == -1) {
            std::cerr << "Error receiving command from client: " << strerror(errno) << std::endl;
            __android_log_print(ANDROID_LOG_INFO, tag, "Error receiving command from client: %s", strerror(errno));
            close(client_socket_fd);
            continue;
        }
        __android_log_print(ANDROID_LOG_INFO, tag, "%s", "sever debug 2");

        // Handle the "hello" command by sending a "hi" response
        if (std::strcmp(buffer, "hello") == 0) {
            const char *response = "hi";
            ssize_t bytes_sent = send(client_socket_fd, response, std::strlen(response), 0);
            if (bytes_sent == -1) {
                std::cerr << "Error sending response to client: " << strerror(errno) << std::endl;
                __android_log_print(ANDROID_LOG_INFO, tag, "Error sending response to client: %s", strerror(errno));
            }
        }

        // Close the client socket
        close(client_socket_fd);
    }

    // Close the server socket (this code is unreachable)
    close(server_socket_fd);
    return 0;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_socket_MainActivity_talkToServer(JNIEnv *env, jobject /* this */) {
    struct sockaddr_un server_address;
    int client_socket_fd = -1;
    char buffer[256] = {0};

    // Create a new socket
    client_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_socket_fd == -1) {
        std::cerr << "Error creating client socket: " << strerror(errno) << std::endl;
        return env->NewStringUTF("");
    }

    // Set the socket address to the server's abstract socket address
    std::memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    std::strcpy(server_address.sun_path, "\0mysocket");
    __android_log_print(ANDROID_LOG_INFO, tag, "%s", "client debug 1");
    // Connect to the server
    if (connect(client_socket_fd, (struct sockaddr *)&server_address, sizeof(sa_family_t) + strlen(server_address.sun_path)) == -1) {
        std::cerr << "Error connecting to server: " << strerror(errno) << std::endl;
        close(client_socket_fd);
        __android_log_print(ANDROID_LOG_INFO, tag, "client debug Error connecting to server: %s", strerror(errno));
        return env->NewStringUTF("");
    }

    __android_log_print(ANDROID_LOG_INFO, tag, "%s", "client debug 2");

    // Send the "hello" command to the server
    const char *command = "hello";
    ssize_t bytes_sent = send(client_socket_fd, command, std::strlen(command), 0);
    if (bytes_sent == -1) {
        std::cerr << "Error sending command to server: " << strerror(errno) << std::endl;
        close(client_socket_fd);
        return env->NewStringUTF("");
    }

    // Wait for the server's response
    ssize_t bytes_received = recv(client_socket_fd, buffer, sizeof(buffer), 0);
    if (bytes_received == -1) {
        std::cerr << "Error receiving response from server: " << strerror(errno) << std::endl;
        close(client_socket_fd);
        return env->NewStringUTF("");
    }

    // Close the client socket
    close(client_socket_fd);

    // Return the response string as a jstring
    return env->NewStringUTF(buffer);
}