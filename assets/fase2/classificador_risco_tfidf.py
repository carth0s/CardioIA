
from __future__ import annotations

import csv
from pathlib import Path

import pandas as pd
from sklearn.feature_extraction.text import TfidfVectorizer
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
from sklearn.model_selection import cross_val_score, train_test_split
from sklearn.pipeline import Pipeline


BASE_DIR = Path(__file__).resolve().parent
CSV_PATH = BASE_DIR / "base_risco_triagem.csv"
TXT_TESTE_PATH = BASE_DIR / "relatos_pacientes.txt"
RELATORIO_PATH = BASE_DIR / "relatorio_classificador_risco.txt"
PREDICOES_PATH = BASE_DIR / "predicoes_risco_exemplos.csv"

STOPWORDS = {
    "a", "o", "os", "as", "de", "da", "do", "das", "dos", "e", "em", "no", "na",
    "nos", "nas", "um", "uma", "uns", "umas", "com", "sem", "por", "para", "ao",
    "aos", "que", "mas", "desde", "depois", "antes", "isso", "esta", "estou",
    "tenho", "sinto", "senti", "tive", "meu", "minha", "meus", "minhas"
}



def carregar_base(csv_path: Path) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    colunas_necessarias = {"frase", "situacao"}
    if not colunas_necessarias.issubset(df.columns):
        raise ValueError(
            f"O arquivo {csv_path.name} precisa conter as colunas: frase, situacao"
        )
    return df.dropna(subset=["frase", "situacao"])


def ler_frases_teste(txt_path: Path) -> list[str]:
    if txt_path.exists():
        with open(txt_path, "r", encoding="utf-8") as f:
            frases = [linha.strip() for linha in f if linha.strip()]
        if frases:
            return frases

    # Fallback caso o TXT não esteja disponível
    return [
        "Sinto dor no peito e suor frio quando caminho rápido.",
        "Estou com coriza e nariz entupido desde ontem.",
        "Tive desmaio e falta de ar hoje de manhã.",
        "Tenho uma dor leve nas costas depois de trabalhar sentado.",
        "Estou com febre alta e dificuldade para respirar.",
        "Sinto azia depois das refeições.",
        "Minha fala ficou enrolada e meu braço esquerdo ficou fraco.",
        "Estou com tosse leve e garganta irritada.",
        "Tenho palpitações com tontura e quase desmaiei.",
        "Sinto uma fisgada no pescoço quando viro a cabeça.",
    ]


def criar_pipeline() -> Pipeline:
    return Pipeline(
        steps=[
            (
                "tfidf",
                TfidfVectorizer(
                    lowercase=True,
                    strip_accents="unicode",
                    ngram_range=(1, 2),
                    min_df=1,
                ),
            ),
            (
                "modelo",
                LogisticRegression(
                    max_iter=2000,
                    class_weight="balanced",
                    random_state=42,
                ),
            ),
        ]
    )


def top_termos(modelo: Pipeline, n: int = 10) -> tuple[list[tuple[str, float]], list[tuple[str, float]]]:
    vetor = modelo.named_steps["tfidf"]
    clf = modelo.named_steps["modelo"]
    termos = vetor.get_feature_names_out()
    pesos = clf.coef_[0]

    # Em classificação binária do scikit-learn, coef_[0] favorece classes_[1].
    # Ajustamos os sinais para que o score positivo sempre signifique maior associação à classe indicada.
    classe_positiva = clf.classes_[1]

    if classe_positiva == "alto risco":
        score_alto = pesos
        score_baixo = -pesos
    else:
        score_alto = -pesos
        score_baixo = pesos

    def termo_relevante(termo: str) -> bool:
        tokens = termo.split()
        if not tokens:
            return False
        if all((tok in STOPWORDS or len(tok) <= 2) for tok in tokens):
            return False
        return True

    pares_alto = [(t, s) for t, s in zip(termos, score_alto) if termo_relevante(t)]
    pares_baixo = [(t, s) for t, s in zip(termos, score_baixo) if termo_relevante(t)]

    alto_risco = sorted(pares_alto, key=lambda x: x[1], reverse=True)[:n]
    baixo_risco = sorted(pares_baixo, key=lambda x: x[1], reverse=True)[:n]
    return alto_risco, baixo_risco


def analisar_erros(y_true, y_pred, frases_teste):
    erros = []
    for frase, real, pred in zip(frases_teste, y_true, y_pred):
        if real != pred:
            erros.append((frase, real, pred))
    return erros


def salvar_predicoes(frases: list[str], preds: list[str], probs_alto: list[float], destino: Path) -> None:
    with open(destino, "w", newline="", encoding="utf-8-sig") as f:
        writer = csv.writer(f)
        writer.writerow(["frase", "predicao", "probabilidade_alto_risco"])
        for frase, pred, prob in zip(frases, preds, probs_alto):
            writer.writerow([frase, pred, round(float(prob), 4)])


def main() -> None:
    df = carregar_base(CSV_PATH)
    X = df["frase"]
    y = df["situacao"]

    X_train, X_test, y_train, y_test = train_test_split(
        X,
        y,
        test_size=0.25,
        random_state=42,
        stratify=y,
    )

    pipeline = criar_pipeline()
    pipeline.fit(X_train, y_train)

    y_pred = pipeline.predict(X_test)
    acc = accuracy_score(y_test, y_pred)
    matriz = confusion_matrix(y_test, y_pred, labels=["baixo risco", "alto risco"])
    relatorio = classification_report(y_test, y_pred, digits=4)
    cv_scores = cross_val_score(pipeline, X, y, cv=5, scoring="accuracy")

    frases_exemplo = ler_frases_teste(TXT_TESTE_PATH)
    preds_exemplo = pipeline.predict(frases_exemplo)
    probs = pipeline.predict_proba(frases_exemplo)
    classes = list(pipeline.named_steps["modelo"].classes_)
    indice_alto = classes.index("alto risco")
    probs_alto = probs[:, indice_alto]

    salvar_predicoes(frases_exemplo, preds_exemplo, probs_alto, PREDICOES_PATH)

    termos_alto, termos_baixo = top_termos(pipeline, n=10)
    erros = analisar_erros(list(y_test), list(y_pred), list(X_test))

    interpretacao = []
    interpretacao.append("Interpretação resumida:")
    interpretacao.append(
        "- O modelo tende a associar palavras como 'falta de ar', 'peito', 'desmaio', 'sudorese' e 'confusão' a ALTO RISCO."
    )
    interpretacao.append(
        "- Expressões como 'leve', 'incômodo', 'coriza', 'espirros', 'azia' e 'muscular' empurram a previsão para BAIXO RISCO."
    )
    interpretacao.append(
        "- Como a base é pequena e simulada, a acurácia pode parecer alta demais e não representa desempenho clínico real."
    )
    interpretacao.append(
        "- O modelo aprende muito por palavras-chave; frases ambíguas ou com negações podem ser classificadas de forma inadequada."
    )

    with open(RELATORIO_PATH, "w", encoding="utf-8") as f:
        f.write("CLASSIFICADOR BÁSICO DE RISCO CLÍNICO (SIMULAÇÃO)\n")
        f.write("=" * 60 + "\n\n")
        f.write(f"Total de exemplos na base: {len(df)}\n")
        f.write(f"Treino: {len(X_train)} | Teste: {len(X_test)}\n")
        f.write(f"Acurácia no conjunto de teste: {acc:.4f}\n")
        f.write(
            f"Acurácia média em validação cruzada (5 folds): {cv_scores.mean():.4f} "
            f"(desvio padrão: {cv_scores.std():.4f})\n\n"
        )

        f.write("Matriz de confusão [linhas = real, colunas = previsto]\n")
        f.write("Classes: [baixo risco, alto risco]\n")
        f.write(f"{matriz}\n\n")

        f.write("Relatório de classificação\n")
        f.write(relatorio + "\n")

        f.write("Termos mais associados a ALTO RISCO\n")
        for termo, peso in termos_alto:
            f.write(f"- {termo}: {peso:.4f}\n")
        f.write("\n")

        f.write("Termos mais associados a BAIXO RISCO\n")
        for termo, peso in termos_baixo:
            f.write(f"- {termo}: {peso:.4f}\n")
        f.write("\n")

        f.write("Erros observados no conjunto de teste\n")
        if erros:
            for frase, real, pred in erros:
                f.write(f"- Frase: {frase}\n  Real: {real} | Previsto: {pred}\n")
        else:
            f.write("- Nenhum erro neste particionamento específico.\n")
        f.write("\n")

        f.write("\n".join(interpretacao))
        f.write("\n\nPredições em frases de exemplo salvas em: predicoes_risco_exemplos.csv\n")
        f.write("Observação importante: este projeto é apenas educacional e não deve ser usado em decisões reais de saúde.\n")

    print(f"Arquivo de base: {CSV_PATH}")
    print(f"Relatório salvo em: {RELATORIO_PATH}")
    print(f"Predições de exemplo salvas em: {PREDICOES_PATH}")
    print(f"Acurácia teste: {acc:.4f}")
    print(f"Acurácia CV média: {cv_scores.mean():.4f}")


if __name__ == "__main__":
    main()
