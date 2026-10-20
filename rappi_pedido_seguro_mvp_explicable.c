#include <stdio.h>
#include <string.h>

#define MAX_PEDIDOS 50
#define ARCHIVO_DATOS "pedidos_seguro.dat"
#define ARCHIVO_REPORTE "reporte_pedido_seguro.txt"

typedef enum {
    SELLO_SIN_VERIFICAR = 0,
    SELLO_INTACTO = 1,
    SELLO_ROTO = 2,
    SELLO_ABIERTO = 3
} EstadoSello;

typedef enum {
    PEDIDO_CREADO = 1,
    PEDIDO_SELLADO = 2,
    PEDIDO_RECOGIDO = 3,
    PEDIDO_CONFORME = 4,
    PEDIDO_OBSERVADO = 5
} EstadoPedido;

typedef struct {
    int id;
    char comercio[50];
    char cliente[50];
    char repartidor[50];
    char productos[120];

    char selloComercio[20];
    char selloRepartidor[20];
    char selloCliente[20];

    EstadoSello estadoComercio;
    EstadoSello estadoRepartidor;
    EstadoSello estadoCliente;
    EstadoPedido estadoPedido;

    char incidencia[160];
} Pedido;

Pedido pedidos[MAX_PEDIDOS];
int cantidadPedidos = 0;

void limpiarEntrada(void) {
    int caracter;
    while ((caracter = getchar()) != '\n' && caracter != EOF) { }
}

void leerTexto(const char *mensaje, char texto[], int tamano) {
    printf("%s", mensaje);
    if (fgets(texto, tamano, stdin) != NULL) {
        texto[strcspn(texto, "\n")] = '\0';
    }
}

const char *textoEstadoSello(EstadoSello estado) {
    switch (estado) {
        case SELLO_INTACTO: return "INTACTO";
        case SELLO_ROTO: return "ROTO";
        case SELLO_ABIERTO: return "ABIERTO";
        default: return "SIN VERIFICAR";
    }
}

const char *textoEstadoPedido(EstadoPedido estado) {
    switch (estado) {
        case PEDIDO_CREADO: return "CREADO";
        case PEDIDO_SELLADO: return "SELLADO";
        case PEDIDO_RECOGIDO: return "RECOGIDO";
        case PEDIDO_CONFORME: return "CONFORME";
        case PEDIDO_OBSERVADO: return "OBSERVADO";
        default: return "DESCONOCIDO";
    }
}

EstadoSello leerEstadoSello(void) {
    int opcion;

    printf("1. Intacto\n");
    printf("2. Roto\n");
    printf("3. Abierto\n");
    printf("Seleccione el estado: ");

    if (scanf("%d", &opcion) != 1) {
        limpiarEntrada();
        return SELLO_SIN_VERIFICAR;
    }
    limpiarEntrada();

    if (opcion == 1) return SELLO_INTACTO;
    if (opcion == 2) return SELLO_ROTO;
    if (opcion == 3) return SELLO_ABIERTO;
    return SELLO_SIN_VERIFICAR;
}

Pedido *buscarPedido(int id) {
    int i;
    for (i = 0; i < cantidadPedidos; i++) {
        if (pedidos[i].id == id) {
            return &pedidos[i];
        }
    }
    return NULL;
}

void crearPedido(void) {
    Pedido *pedido;

    if (cantidadPedidos >= MAX_PEDIDOS) {
        printf("No se pueden registrar mas pedidos.\n");
        return;
    }

    pedido = &pedidos[cantidadPedidos];
    memset(pedido, 0, sizeof(Pedido));

    pedido->id = 1001 + cantidadPedidos;
    leerTexto("Nombre del comercio: ", pedido->comercio, sizeof(pedido->comercio));
    leerTexto("Nombre del cliente: ", pedido->cliente, sizeof(pedido->cliente));
    leerTexto("Productos del pedido: ", pedido->productos, sizeof(pedido->productos));

    if (strlen(pedido->comercio) == 0 || strlen(pedido->cliente) == 0) {
        printf("El comercio y el cliente son obligatorios.\n");
        return;
    }

    pedido->estadoPedido = PEDIDO_CREADO;
    cantidadPedidos++;

    printf("Pedido RAP-%d creado correctamente.\n", pedido->id);
}

void registrarComercio(void) {
    int id;
    Pedido *pedido;

    printf("ID numerico del pedido: ");
    if (scanf("%d", &id) != 1) {
        limpiarEntrada();
        return;
    }
    limpiarEntrada();

    pedido = buscarPedido(id);
    if (pedido == NULL) {
        printf("Pedido no encontrado.\n");
        return;
    }
    if (pedido->estadoPedido != PEDIDO_CREADO) {
        printf("El pedido ya fue procesado por el comercio.\n");
        return;
    }

    leerTexto("Codigo del sello: ", pedido->selloComercio, sizeof(pedido->selloComercio));
    if (strlen(pedido->selloComercio) == 0) {
        printf("El codigo no puede estar vacio.\n");
        return;
    }

    printf("Estado inicial del sello:\n");
    pedido->estadoComercio = leerEstadoSello();
    pedido->estadoPedido = PEDIDO_SELLADO;

    printf("Registro del comercio guardado.\n");
}

void verificarRepartidor(void) {
    int id;
    Pedido *pedido;

    printf("ID numerico del pedido: ");
    if (scanf("%d", &id) != 1) {
        limpiarEntrada();
        return;
    }
    limpiarEntrada();

    pedido = buscarPedido(id);
    if (pedido == NULL) {
        printf("Pedido no encontrado.\n");
        return;
    }
    if (pedido->estadoPedido != PEDIDO_SELLADO) {
        printf("El comercio debe registrar el sello primero.\n");
        return;
    }

    leerTexto("Nombre del repartidor: ", pedido->repartidor, sizeof(pedido->repartidor));
    leerTexto("Codigo visible del sello: ", pedido->selloRepartidor, sizeof(pedido->selloRepartidor));
    printf("Estado del sello durante el recojo:\n");
    pedido->estadoRepartidor = leerEstadoSello();

    if (strcmp(pedido->selloComercio, pedido->selloRepartidor) != 0 ||
        pedido->estadoRepartidor != SELLO_INTACTO) {
        pedido->estadoPedido = PEDIDO_OBSERVADO;
        strcpy(pedido->incidencia,
               "Primera inconsistencia detectada durante el recojo.");
        printf("RECOJO OBSERVADO. El traslado debe detenerse.\n");
    } else {
        pedido->estadoPedido = PEDIDO_RECOGIDO;
        printf("RECOJO CONFORME. El traslado puede comenzar.\n");
    }
}

void confirmarCliente(void) {
    int id;
    int completo;
    Pedido *pedido;

    printf("ID numerico del pedido: ");
    if (scanf("%d", &id) != 1) {
        limpiarEntrada();
        return;
    }
    limpiarEntrada();

    pedido = buscarPedido(id);
    if (pedido == NULL) {
        printf("Pedido no encontrado.\n");
        return;
    }
    if (pedido->estadoPedido != PEDIDO_RECOGIDO) {
        printf("El pedido no esta listo para confirmar la entrega.\n");
        return;
    }

    leerTexto("Codigo recibido: ", pedido->selloCliente, sizeof(pedido->selloCliente));
    printf("Estado final del sello:\n");
    pedido->estadoCliente = leerEstadoSello();

    printf("El pedido esta completo? (1 = Si, 0 = No): ");
    if (scanf("%d", &completo) != 1) {
        limpiarEntrada();
        completo = 0;
    } else {
        limpiarEntrada();
    }

    if (strcmp(pedido->selloComercio, pedido->selloCliente) == 0 &&
        pedido->estadoCliente == SELLO_INTACTO && completo == 1) {
        pedido->estadoPedido = PEDIDO_CONFORME;
        strcpy(pedido->incidencia, "Sin incidencias.");
        printf("ENTREGA CONFORME.\n");
    } else {
        pedido->estadoPedido = PEDIDO_OBSERVADO;
        leerTexto("Describa la incidencia: ", pedido->incidencia,
                  sizeof(pedido->incidencia));
        printf("ENTREGA OBSERVADA. El caso debe ser revisado.\n");
    }
}

void consultarCadenaCustodia(void) {
    int id;
    Pedido *pedido;

    printf("ID numerico del pedido: ");
    if (scanf("%d", &id) != 1) {
        limpiarEntrada();
        return;
    }
    limpiarEntrada();

    pedido = buscarPedido(id);
    if (pedido == NULL) {
        printf("Pedido no encontrado.\n");
        return;
    }

    printf("\n========== CADENA DE CUSTODIA ==========\n");
    printf("Pedido: RAP-%d\n", pedido->id);
    printf("Comercio: %s\n", pedido->comercio);
    printf("Cliente: %s\n", pedido->cliente);
    printf("Repartidor: %s\n",
           strlen(pedido->repartidor) > 0 ? pedido->repartidor : "No asignado");
    printf("Productos: %s\n", pedido->productos);
    printf("Estado general: %s\n", textoEstadoPedido(pedido->estadoPedido));

    printf("\nComercio    | Codigo: %-10s | Estado: %s\n",
           strlen(pedido->selloComercio) > 0 ? pedido->selloComercio : "-",
           textoEstadoSello(pedido->estadoComercio));
    printf("Repartidor  | Codigo: %-10s | Estado: %s\n",
           strlen(pedido->selloRepartidor) > 0 ? pedido->selloRepartidor : "-",
           textoEstadoSello(pedido->estadoRepartidor));
    printf("Cliente     | Codigo: %-10s | Estado: %s\n",
           strlen(pedido->selloCliente) > 0 ? pedido->selloCliente : "-",
           textoEstadoSello(pedido->estadoCliente));

    printf("Incidencia: %s\n",
           strlen(pedido->incidencia) > 0 ? pedido->incidencia : "Sin registro");
    printf("=========================================\n");
}

void listarPedidos(void) {
    int i;

    if (cantidadPedidos == 0) {
        printf("No hay pedidos registrados.\n");
        return;
    }

    printf("\n========== LISTA DE PEDIDOS ==========\n");
    for (i = 0; i < cantidadPedidos; i++) {
        printf("RAP-%d | %-20s | %s\n",
               pedidos[i].id,
               pedidos[i].comercio,
               textoEstadoPedido(pedidos[i].estadoPedido));
    }
}

void generarReporte(void) {
    FILE *archivo;
    int i;

    archivo = fopen(ARCHIVO_REPORTE, "w");
    if (archivo == NULL) {
        printf("No se pudo crear el reporte.\n");
        return;
    }

    fprintf(archivo, "RAPPI PEDIDO SEGURO - REPORTE MVP\n\n");
    for (i = 0; i < cantidadPedidos; i++) {
        fprintf(archivo, "RAP-%d | %s | %s | %s\n",
                pedidos[i].id,
                pedidos[i].comercio,
                textoEstadoPedido(pedidos[i].estadoPedido),
                strlen(pedidos[i].incidencia) > 0
                    ? pedidos[i].incidencia
                    : "Sin incidencia registrada");
    }

    fclose(archivo);
    printf("Reporte generado: %s\n", ARCHIVO_REPORTE);
}

void guardarDatos(void) {
    FILE *archivo = fopen(ARCHIVO_DATOS, "wb");

    if (archivo == NULL) {
        printf("No se pudieron guardar los datos.\n");
        return;
    }

    fwrite(&cantidadPedidos, sizeof(int), 1, archivo);
    fwrite(pedidos, sizeof(Pedido), cantidadPedidos, archivo);
    fclose(archivo);
}

void cargarDatos(void) {
    FILE *archivo = fopen(ARCHIVO_DATOS, "rb");

    if (archivo == NULL) {
        return;
    }

    if (fread(&cantidadPedidos, sizeof(int), 1, archivo) != 1 ||
        cantidadPedidos < 0 || cantidadPedidos > MAX_PEDIDOS) {
        cantidadPedidos = 0;
        fclose(archivo);
        return;
    }

    fread(pedidos, sizeof(Pedido), cantidadPedidos, archivo);
    fclose(archivo);
}

void mostrarMenu(void) {
    printf("\n========================================\n");
    printf(" RAPPI PEDIDO SEGURO - MVP EN C\n");
    printf("========================================\n");
    printf("1. Crear pedido\n");
    printf("2. Registrar sello del comercio\n");
    printf("3. Verificar sello del repartidor\n");
    printf("4. Confirmar entrega del cliente\n");
    printf("5. Consultar cadena de custodia\n");
    printf("6. Listar pedidos\n");
    printf("7. Generar reporte\n");
    printf("8. Guardar y salir\n");
    printf("Seleccione una opcion: ");
}

int main(void) {
    int opcion = 0;

    cargarDatos();

    do {
        mostrarMenu();

        if (scanf("%d", &opcion) != 1) {
            limpiarEntrada();
            printf("Entrada no valida.\n");
            continue;
        }
        limpiarEntrada();

        switch (opcion) {
            case 1: crearPedido(); break;
            case 2: registrarComercio(); break;
            case 3: verificarRepartidor(); break;
            case 4: confirmarCliente(); break;
            case 5: consultarCadenaCustodia(); break;
            case 6: listarPedidos(); break;
            case 7: generarReporte(); break;
            case 8:
                guardarDatos();
                printf("Datos guardados. Programa finalizado.\n");
                break;
            default:
                printf("Seleccione una opcion del 1 al 8.\n");
        }
    } while (opcion != 8);

    return 0;
}
