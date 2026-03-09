# FIAP - Faculdade de Informática e Administração Paulista

<p align="center">
<a href= "https://www.fiap.com.br/"><img src="assets/logo-fiap.png" alt="FIAP - Faculdade de Informática e Admnistração Paulista" border="0" width=40% height=40%></a>
</p>

<br>

# CardioIA

## 👨‍🎓 Integrantes: 

- Carlos Daniel Silveira do Nascimento - RM88439
- Henrique Ribeiro Siqueira - RM565044
- Mauricio Jose Ferlin Tonnera - RM565469
- Rodrigo Portugal Santos - RM564773


## 👩‍🏫 Professores:
### Tutor(a) 
- Caique Nonato da Silva Bezerra
### Coordenador(a)
- André Godoi Chiovato


## 📜 Descrição

Nesta primeira etapa do projeto CardioIA, estabelecemos a base de dados necessária para a construção de um ecossistema inteligente de saúde cardiovascular. Os dados foram selecionados com foco na interoperabilidade e estão divididos em três pilares fundamentais:

### Parte 1 - Dados Numéricos (IoT)
Os dados utilizados no projeto são *reais* e provenientes de registros clínicos anonimizados, respeitando os princípios de Governança de Dados e privacidade. As variáveis mais relevantes incluem pressão arterial (sistólica e diastólica), IMC, idade e histórico de tabagismo, pois são biomarcadores críticos para o desenvolvimento de hipertensão e doenças coronarianas. No ecossistema do CardioIA, esses dados simulam informações coletadas via dispositivos IoT (como smartwatches e tensiômetros digitais), sendo fundamentais para treinar modelos de Machine Learning capazes de realizar a predição de risco cardíaco e apoiar o monitoramento clínico preventivo.

### Parte 2 – Dados Textuais (NLP)
Utilizamos textos técnicos e diretrizes oficiais da OPAS, SUS e IPEMED que abordam patologias cardiovasculares e protocolos de tratamento. Estes dados permitem que algoritmos de Processamento de Linguagem Natural (NLP) realizem a extração de entidades (sintomas e diagnósticos) e a classificação de tópicos em prontuários médicos. A análise desses textos é crucial para correlacionar a literatura científica com os sintomas relatados pelos pacientes, permitindo que a IA compreenda o contexto clínico além dos números, auxiliando na triagem e na tomada de decisão médica.

### Parte 3 – Dados Visuais (VC)
Para o pilar de Visão Computacional, selecionamos 100 imagens de exames de eletrocardiograma (ECG), (extraídas da plataforma Kaggle), distribuídas entre batimentos normais (N) e diferentes tipos de arritmias (V, S, F). Estas imagens permitem que redes neurais identifiquem padrões morfológicos na atividade elétrica do coração. A análise automatizada desses sinais é vital para o diagnóstico de precisão, permitindo que a IA detecte anomalias (como contrações ventriculares prematuras) de forma instantânea, servindo como uma ferramenta de suporte rápido para cardiologistas em cenários de alta complexidade.

- Os arquivos referentes aos dados coletados podem ser acessados tanto na pasta assets quanto por este link: <a href="https://drive.google.com/drive/folders/1SQ8nB5iq25YXGnu0UCDY_LQu02pGcsuy"> Drive </a>


## 📁 Estrutura de pastas

Dentre os arquivos e pastas presentes na raiz do projeto, definem-se:

- <b>assets</b>: aqui estão os arquivos relacionados ao projeto.

- <b>README.md</b>: arquivo que serve como guia e explicação geral sobre o projeto (o mesmo que você está lendo agora).


## 📋 Licença

<img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/cc.svg?ref=chooser-v1"><img style="height:22px!important;margin-left:3px;vertical-align:text-bottom;" src="https://mirrors.creativecommons.org/presskit/icons/by.svg?ref=chooser-v1"><p xmlns:cc="http://creativecommons.org/ns#" xmlns:dct="http://purl.org/dc/terms/"><a property="dct:title" rel="cc:attributionURL" href="https://github.com/agodoi/template">MODELO GIT FIAP</a> por <a rel="cc:attributionURL dct:creator" property="cc:attributionName" href="https://fiap.com.br">Fiap</a> está licenciado sobre <a href="http://creativecommons.org/licenses/by/4.0/?ref=chooser-v1" target="_blank" rel="license noopener noreferrer" style="display:inline-block;">Attribution 4.0 International</a>.</p>
