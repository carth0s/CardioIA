import csv
import unicodedata
from collections import defaultdict
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent
ARQUIVO_FRASES = BASE_DIR / "relatos_pacientes.txt"
ARQUIVO_MAPA = BASE_DIR / "mapa_sintomas_doencas.csv"


def normalizar_texto(texto: str) -> str:
    texto = texto.lower().strip()
    texto = unicodedata.normalize("NFD", texto)
    texto = "".join(c for c in texto if unicodedata.category(c) != "Mn")
    return texto


def carregar_mapa(caminho_csv: Path):
    mapa = []
    with caminho_csv.open("r", encoding="utf-8-sig", newline="") as arquivo:
        leitor = csv.DictReader(arquivo)
        for linha in leitor:
            mapa.append(
                {
                    "sintoma_1": linha["Sintoma 1"].strip(),
                    "sintoma_2": linha["Sintoma 2"].strip(),
                    "doenca": linha["Doença Associada"].strip(),
                }
            )
    return mapa


def diagnosticar(frase: str, mapa):
    frase_normalizada = normalizar_texto(frase)
    sintomas_por_doenca = defaultdict(set)

    for item in mapa:
        doenca = item["doenca"]
        for chave in ("sintoma_1", "sintoma_2"):
            sintoma = item[chave]
            sintoma_normalizado = normalizar_texto(sintoma)
            if sintoma_normalizado and sintoma_normalizado in frase_normalizada:
                sintomas_por_doenca[doenca].add(sintoma)

    if not sintomas_por_doenca:
        return {
            "diagnostico": "Nenhum diagnóstico sugerido",
            "pontuacao": 0,
            "sintomas": [],
        }

    pontuacoes = {doenca: len(sintomas) for doenca, sintomas in sintomas_por_doenca.items()}
    maior_pontuacao = max(pontuacoes.values())
    melhores_doencas = sorted(
        [doenca for doenca, pontos in pontuacoes.items() if pontos == maior_pontuacao]
    )

    if len(melhores_doencas) == 1:
        diagnostico = melhores_doencas[0]
        sintomas = sorted(sintomas_por_doenca[diagnostico])
    else:
        diagnostico = " / ".join(melhores_doencas)
        sintomas = sorted(
            {
                sintoma
                for doenca in melhores_doencas
                for sintoma in sintomas_por_doenca[doenca]
            }
        )

    return {
        "diagnostico": diagnostico,
        "pontuacao": maior_pontuacao,
        "sintomas": sintomas,
    }


def main():
    mapa = carregar_mapa(ARQUIVO_MAPA)

    with ARQUIVO_FRASES.open("r", encoding="utf-8") as arquivo:
        frases = [linha.strip() for linha in arquivo if linha.strip()]

    print("Sistema básico de apoio ao diagnóstico")
    print("-" * 50)
    print("Aviso: este resultado é apenas educacional e não substitui avaliação médica.\n")

    for indice, frase in enumerate(frases, start=1):
        resultado = diagnosticar(frase, mapa)
        sintomas_texto = ", ".join(resultado["sintomas"]) if resultado["sintomas"] else "nenhum"
        print(f"Paciente {indice}:")
        print(f"Relato: {frase}")
        print(f"Sintomas identificados: {sintomas_texto}")
        print(f"Possível diagnóstico: {resultado['diagnostico']}")
        print(f"Pontuação: {resultado['pontuacao']}")
        print("-" * 50)


if __name__ == "__main__":
    main()
