# IoT Trade-Off Study

## Resumo do projeto

Este projeto implementa uma simulação NS-3 para comparar três protocolos de comunicação de IoT (MQTT, CoAP e AMQP) em combinações com três modos de segurança (NONE, TLS e MTLS). Ele modela um cenário de rede Wi-Fi 802.11g com um gateway estático e um conjunto de nós sensores que enviam dados periódicos. A coleta de métricas é feita no próprio simulador: estatísticas de fluxo agregadas pelo FlowMonitor, contagem de mensagens no nível de aplicação e medição local de custo de CPU por operação.

## Estrutura do repositório

- `main.cc`: implementa o cenário NS-3, configura a topologia, instala aplicações de sensor e gateway, coleta métricas e imprime os resultados.
- `IoTSensorApplication.cc` / `IoTSensorApplication.h`: define o comportamento dos nós sensores, incluindo inicialização de protocolo, handshake de segurança e envio periódico de pacotes.
- `IoTGatewayApplication.cc` / `IoTGatewayApplication.h`: implementa a recepção de pacotes no gateway, a extração do cabeçalho IoT e o cálculo do atraso de aplicação e mensagens perdidas.
- `CommunicationProfile.h`, `CommunicationMessageSizes.h`: definem perfis de protocolo de comunicação e os tamanhos de mensagem usados para estimar overhead de controle.
- `SecurityProfile.h`, `SecurityMessage.h`: definem perfis de segurança e os tamanhos de mensagens de handshake TLS/MTLS.
- `CpuModel.h`, `CpuModel.cc`, `CpuProfile.h`: modelam custo de CPU estimado para transmissão, operações de protocolo e operações de segurança.
- `Scenario.h`: contém os parâmetros padrão de cenário, incluindo número de nós, duração da simulação e tamanho do payload.
- `run_experiments.py`: script de automação que executa combinações de protocolo, segurança e número de nós e grava resultados em CSV.
- `experiment_results.csv`: amostra de resultados provenientes de uma execução anterior.

## Como o projeto funciona

A simulação é iniciada em `main.cc`. O fluxo principal do código é:

1. Leitura de argumentos de linha de comando: `--protocol`, `--security`, `--nodes`, `--duration` e `--payload`.
2. Criação de um nó gateway e de `params.nodes` nós sensores.
3. Configuração de rede Wi-Fi 802.11g com um AP único e nós sensores em posições fixas.
4. Instalação da pilha de protocolos IP e atribuição de endereços IPv4.
5. Instalação de um `PacketSink` no gateway para receber tráfego de sensores.
6. Criação e instalação de instâncias de `IoTSensorApplication` em cada sensor.
7. Coleta de métricas de fluxo com `FlowMonitor` e métricas de aplicação por meio de rastreamento do pacote recebido.
8. Impressão em stdout de métricas de rede, aplicação e CPU.

O script `run_experiments.py` invoca o executável compilado, parseia seu stdout e grava cada combinação de experimento em `experiment_results.csv`.

## Métricas coletadas e como são obtidas

### Métricas de rede

- `TX Packets`: soma de `flow.second.txPackets` de todos os fluxos reportados pelo FlowMonitor. Representa o número de pacotes transmitidos pela camada de rede durante a simulação.
- `RX Packets`: soma de `flow.second.rxPackets`, ou seja, o número de pacotes recebidos corretamente pelo destino segundo o FlowMonitor.
- `Lost Packets`: soma de `flow.second.lostPackets`. Refere-se a pacotes perdidos detectados pelo FlowMonitor entre transmissões e recepções na camada de rede.
- `RX Bytes`: soma de `flow.second.rxBytes`, o total de bytes recebidos no destino segundo o FlowMonitor.
- `Throughput`: calculado como `(RX Bytes * 8) / Duration / 1000`, em kbps. O denominador é a duração de simulação configurada; não usa valores de tempo de execução do sistema operacional.
- `Average Delay`: calculado como `delaySum / RX Packets`, em milissegundos. `delaySum` é a soma de atrasos relatados pelo FlowMonitor para todos os pacotes recebidos.
- `Average Jitter`: calculado como `jitterSum / (RX Packets - 1)`, em milissegundos. O FlowMonitor fornece `jitterSum` agregado para todos os fluxos.
- `Simulation CPU Time`: tempo de CPU medido localmente em `main.cc` com `std::chrono::steady_clock` entre `Simulator::Run()` e o final da simulação. É uma medida do tempo de execução do processo no host, não do consumo real de CPU de cada nó simulado.

### Métricas de aplicação

- `Messages Sent`: soma de `IoTSensorApplication::GetMessagesSent()` para todos os sensores. Incrementado toda vez que um pacote de dados IoT é enviado.
- `Messages Received`: obtido de `IoTGatewayApplication::GetMessagesReceived()`. Incrementado pelo gateway ao processar um pacote com cabeçalho IoT válido.
- `Messages Lost`: estimado em `IoTGatewayApplication::UpdateSequenceTracking()` usando a diferença entre números de sequência consecutivos de sensor. Esta métrica reflete lacunas de sequência detectadas no gateway, não uma confirmação end-to-end real.
- `Average Application Delay`: atraso médio calculado em `IoTGatewayApplication::HandlePacketTrace()` como `receiveTime - timestamp`, onde `timestamp` vem do cabeçalho IoT do pacote e `receiveTime` é o tempo simulado no momento da recepção.
- `Security Overhead Bytes`: soma de overhead de segurança acumulada em `IoTSensorApplication`. Inclui bytes adicionados ao pacote de dados por TLS/MTLS e bytes de handshake de segurança.
- `Security Handshake Messages`: número total de mensagens de handshake TLS/MTLS enviadas por `IoTSensorApplication` durante a inicialização.

### Métricas de CPU e custo de operação

O projeto contém uma modelagem interna de custo de CPU, mas os coeficientes de custo não estão calibrados para hardware real. Por isso, os resultados atuais não expõem métricas de custo de CPU derivadas dessa modelagem.

## Como rodar o experimento

O script `run_experiments.py` espera que o binário NS-3 esteja disponível em `../build/scratch/iot-study/ns3-dev-main-default` a partir do diretório do script.

Exemplo de execução manual:

```bash
cd /ns-3-dev/scratch/iot-study
python3 run_experiments.py
```

Se a compilação não existir, o script lança `FileNotFoundError` e informa o caminho esperado.

## Parâmetros de entrada significativos

- `--protocol`: `MQTT`, `COAP` ou `AMQP`.
- `--security`: `NONE`, `TLS` ou `MTLS`.
- `--nodes`: número de nós sensores. No `run_experiments.py`, atualmente o conjunto de testes usa `5, 10, 20` para execuções de validação rápidas; o código de cenário usa `ScenarioParameters::nodes`, que por padrão é `200`.
- `--duration`: duração da simulação em segundos. O código padrão usa `300.0`.
- `--payload`: tamanho do payload UDP em bytes. O padrão é `256`.
- `--seed`: valor inteiro que controla o gerador de números aleatórios do NS-3. O script `run_experiments.py` atribui um valor diferente a cada execução.

## Limitações e escopo de validade

- O modelo de custo de CPU presente no código não está calibrado para hardware real e não é usado como métrica de análise nos resultados atuais.
- Os valores em `CommunicationMessageSizes.h` não são validados contra formatos completos de pacote de MQTT, CoAP ou AMQP; o cálculo de overhead de controle é um valor de modelo.
- A simulação usa Wi-Fi 802.11g com um único AP e posições fixas; não há mobilidade dinâmica nem múltiplos canais.
- A métrica `Messages Lost` no gateway é inferida por sequência de pacotes faltantes e pode não corresponder a perdas reais em cenários com retransmissões ou reordenação.
- `Simulation CPU Time` mede duração de execução no host e não é um indicador direto de carga de CPU por nó simulado ou de performance do protocolo em hardware real.
- Não há repetição estatística automática de experimentos. Cada linha de `experiment_results.csv` representa apenas uma execução de configuração específica. Isso impede inferências robustas sobre variabilidade ou intervalos de confiança.
- O `run_experiments.py` grava combinações de protocolo, segurança e número de nós em ordem aleatória e passa uma semente de simulação explícita ao NS-3. A variação de cada execução depende dos processos aleatórios internos do NS-3, como o backoff do Wi-Fi, mas cada linha ainda identifica apenas uma única semente de simulação.
- A topologia modelada contém apenas um gateway e não representa arquiteturas distribuídas de borda ou múltiplos servidores.
- A implementação de TLS/MTLS considera mensagens de handshake e overhead de aplicação, mas não inclui validação de certificados, renegociação, re-transmissão de handshake ou o custo real de cifragem em bibliotecas de criptografia reais.

## O que estes dados podem suportar

Os resultados servem para comparar comportamentos relativos dentro desta configuração específica de simulação NS-3. Eles são informativos para análises do próprio modelo de simulação, não para concluir como MQTT, CoAP ou AMQP se comportariam em uma rede IoT real sem validação adicional.

Os dados podem ser usados para:

- comparar o impacto relativo de diferentes modos de segurança em uma topologia fixa;
- observar como a escolha de protocolo altera métricas agregadas de fluxo e mensagens no modelo aplicado;
- validar a execução do simulador e a coleta de métricas em um conjunto de experimentos automatizados.

## O que estes dados não devem ser usados para

- fazer afirmações definitivas sobre desempenho real de MQTT/CoAP/AMQP em uma rede Wi-Fi ou de rádio específica;
- estimar consumo de CPU real de dispositivos embarcados sem calibrar `CpuProfile` com dados de hardware real;
- extrapolar métricas para cenários de rede com atributos diferentes (mobilidade, interferência, múltiplos APs, tráfego misto, topologias não-estrela).

## Observações finais

A estrutura do projeto é voltada para comparação entre protocolos e modos de segurança no contexto de um modelo NS-3 específico. A validade dos resultados depende diretamente das suposições de modelagem: custos de CPU abstratos, overhead de mensagem parcialmente definido e uma topologia de rede simplificada.
