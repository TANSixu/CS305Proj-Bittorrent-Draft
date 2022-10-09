#include "spiffy.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <Python.h>
#define MAXLEN 1500

static PyObject * py_spiffy_sendto(PyObject * self, PyObject * args){
    int sock;
    char buf[MAXLEN];
    size_t len;
    int flags;
    char host_name[20];
    int port;
    if (!PyArg_ParseTuple(args, "isiisi", &sock, buf, &len, &flags, host_name, &port)) return NULL;
    
    struct hostent *host;
    struct sockaddr_in addr;
    host = gethostbyname(host_name);
    addr.sin_addr.s_addr = *(in_addr_t *)(host->h_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    ssize_t ret = spiffy_sendto(sock, buf, len, flags, (struct sockaddr*)&addr, sizeof(addr));
    return PyLong_FromSsize_t(ret);
}

static PyMethodDef Methods[] = {
    {"spiffy_sendto", py_spiffy_sendto, METH_VARARGS},
    {NULL, NULL}
};

static struct PyModuleDef cModule = {
    PyModuleDef_HEAD_INIT,
    "Spiffy",
    "",
    -1,
    Methods
};

PyMODINIT_FUNC PyInit_Test(void){return PyModule_Create(&cModule);}