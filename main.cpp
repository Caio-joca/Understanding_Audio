#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#pragma comment(lib, "winmm.a")

using namespace std;

struct audio {
    char chunk_id[4];
    int chunk_size;
    char format[4];
    char subchunk1_id[4];
    int subchunk1_size;
    char audio_format[2];
    short numchannel;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_persample;
    char subchunk2_id[4];
    int subchunk2_size;
} dados;

// Função para ouvir o áudio e permitir que o usuário o interrompa
void ouvirAudio(const char *arquivo) {
    PlaySound(TEXT(arquivo), NULL, SND_ASYNC);
    printf("Pressione 'p' para parar a música e voltar ao menu.\n");
    while (true) {
        if (_kbhit()) {
            char tecla = _getch();
            if (tecla == 'p' || tecla == 'P') {
                PlaySound(NULL, 0, 0); // Para o áudio
                printf("Música parada. Voltando ao menu...\n");
                break;
            }
        }
    }
}

// Função para inverter o áudio
void inverterAudio() {
    FILE *audio1 = fopen("Samba De Orly.wav", "rb");
    if (audio1 == NULL) {
        printf("Erro ao abrir o arquivo de áudio original.\n");
        return;
    }

    fread(&dados, sizeof(dados), 1, audio1);

    // Criar arquivo de saída para o áudio invertido
    FILE *audio2 = fopen("Samba De Orly_invertido.wav", "wb");
    if (audio2 == NULL) {
        printf("Erro ao criar o arquivo de áudio invertido.\n");
        fclose(audio1);
        return;
    }

    // Escrever o cabeçalho no novo arquivo
    fwrite(&dados, sizeof(dados), 1, audio2);

    // Obter o tamanho total dos dados do áudio
    int tamanho_dados = dados.subchunk2_size;
    short *amostras = new short[tamanho_dados / sizeof(short)];

    // Ler todas as amostras no vetor
    fread(amostras, sizeof(short), tamanho_dados / sizeof(short), audio1);

    // Inverter as amostras
    for (int i = 0; i < tamanho_dados / sizeof(short) / 2; i++) {
        short temp = amostras[i];
        amostras[i] = amostras[tamanho_dados / sizeof(short) - 1 - i];
        amostras[tamanho_dados / sizeof(short) - 1 - i] = temp;
    }

    // Gravar as amostras invertidas no novo arquivo
    fwrite(amostras, sizeof(short), tamanho_dados / sizeof(short), audio2);

    // Corrigir o cabeçalho para o novo tamanho dos dados
    dados.subchunk2_size = tamanho_dados; // manter o tamanho igual ao original
    dados.chunk_size = 36 + dados.subchunk2_size;

    // Reabrir o arquivo para atualizar o cabeçalho
    fclose(audio2);
    audio2 = fopen("Samba De Orly_invertido.wav", "rb+");
    if (audio2 != NULL) {
        fwrite(&dados, sizeof(dados), 1, audio2); // Atualiza o cabeçalho
        fclose(audio2);
    }

    delete[] amostras;
    fclose(audio1);

    printf("\nÁudio invertido com sucesso!\n");
}

// Função para alterar o volume do áudio
void alterarVolume() {
    FILE *audio1 = fopen("Samba De Orly.wav", "rb");
    if (audio1 == NULL) {
        printf("Erro ao abrir o arquivo de áudio original.\n");
        return;
    }

    fread(&dados, sizeof(dados), 1, audio1);

    FILE *audio2 = fopen("Samba De Orly_volume.wav", "wb");
    if (audio2 == NULL) {
        printf("Erro ao criar o arquivo de áudio com volume alterado.\n");
        fclose(audio1);
        return;
    }

    fwrite(&dados, sizeof(dados), 1, audio2);

    float volume;
    printf("\nDiga porcentagem de volume a mudar: ");
    scanf("%f", &volume);
    volume = volume / 100;  // Exemplo: 80 -> 0.8

    short sample;
    while (fread(&sample, sizeof(sample), 1, audio1)) {
        sample = sample * volume;  // Aplicando o volume
        fwrite(&sample, sizeof(sample), 1, audio2);
    }

    fclose(audio1);
    fclose(audio2);
    printf("\nVolume alterado e salvo com sucesso!\n");
}

// Função para cortar o áudio
void cortarAudio() {
    FILE *audio1 = fopen("Samba De Orly.wav", "rb");
    if (audio1 == NULL) {
        printf("Erro ao abrir o arquivo de áudio original.\n");
        return;
    }

    fread(&dados, sizeof(dados), 1, audio1);

    // Solicitar tempo inicial e final
    float tempo_inicial, tempo_final;
    printf("\nDigite o tempo inicial (em segundos): ");
    scanf("%f", &tempo_inicial);
    printf("Digite o tempo final (em segundos): ");
    scanf("%f", &tempo_final);


    int amostras_por_segundo = dados.sample_rate * dados.numchannel; //  Calcula o número total de amostras por segundo (taxa de amostragem multiplicada pelo número de canais).
    int inicio_amostra = static_cast<int>(tempo_inicial * amostras_por_segundo); //Multiplica tempo_inicial pelo número de amostras por segundo para obter o índice da amostra correspondente ao tempo inicial.
    int fim_amostra = static_cast<int>(tempo_final * amostras_por_segundo); //Calcula a amostra correspondente ao tempo final
    int num_amostras = fim_amostra - inicio_amostra;


    FILE *audio2 = fopen("Samba De Orly_cortado.wav", "wb");
    if (audio2 == NULL) {
        printf("Erro ao criar o arquivo de áudio cortado.\n");
        fclose(audio1);
        return;
    }

    // Atualizar o cabeçalho para o novo tamanho
    dados.subchunk2_size = num_amostras * sizeof(short); // total de bytes da área de dados
    dados.chunk_size = 36 + dados.subchunk2_size;

    // Escrever o cabeçalho no novo arquivo
    fwrite(&dados, sizeof(dados), 1, audio2);

    // Pular para a amostra inicial
    fseek(audio1, sizeof(dados) + inicio_amostra * sizeof(short), SEEK_SET);

    short sample;
    for (int i = 0; i < num_amostras; i++) {
        fread(&sample, sizeof(sample), 1, audio1);
        fwrite(&sample, sizeof(sample), 1, audio2);
    }

    fclose(audio1);
    fclose(audio2);
    printf("\nÁudio cortado e salvo como 'Samba De Orly_cortado.wav'!\n");
}

int main() {
    int opcao;
    do {
        printf("\nMenu:\n");
        printf("1- Ouvir o áudio original (Samba De Orly)\n");
        printf("2- Cortar o áudio\n");
        printf("3- Ouvir o áudio cortado (Samba De Orly_cortado.wav)\n");
        printf("4- Alterar o volume do áudio\n");
        printf("5- Ouvir o áudio com volume alterado (Samba De Orly_volume.wav)\n");
        printf("6- Inverter o áudio\n");
        printf("7- Ouvir o áudio invertido (Samba De Orly_invertido.wav)\n");
        printf("0- Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                ouvirAudio("Samba De Orly.wav");
                break;
            case 2:
                cortarAudio();
                break;
            case 3:
                ouvirAudio("Samba De Orly_cortado.wav");
                break;
            case 4:
                alterarVolume();
                break;
            case 5:
                ouvirAudio("Samba De Orly_volume.wav");
                break;
            case 6:
                inverterAudio();
                break;
            case 7:
                ouvirAudio("Samba De Orly_invertido.wav");
                break;
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida. Tente novamente.\n");
                break;
        }
    } while (opcao != 0);

    return 0;
}
