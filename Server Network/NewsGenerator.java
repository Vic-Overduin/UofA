import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;

/* NewsGenerator:
* 1) generates a string (news) with modifiable parameters determining:
*		word length
*		sentence length
*		sentences per file
*/

public final class NewsGenerator {

	private static int minSentencesPerFile = 10;
	private static int sentencesRange = 20;
	private static int minWordsPerSentence = 10;
	private static int wordsRange = 30;
	private static int minCharsPerWord = 4;
	private static int charsRange = 16;
	private static int letterAmount = 26;
	private static int numberAmount = 10;
	private static char[] wordPool = new char[letterAmount * 2 + numberAmount];
	private static String[] symbolPool = { ".\r\n\r\n", ". ", ". ", ", ", ",",
	", ", "? ", "! ", ": ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "
	, " ", " ", " ", " ", " ", " ", " ", " " };
	private static int lengthOfWordPool = wordPool.length;
	private static int lengthOfSymbolPool = symbolPool.length;

	private static NewsGenerator rap = null;

	private NewsGenerator() {

	}

	static {
		rap = new NewsGenerator();
		rap.setWordPool();
	}

	private void setWordPool() {
		int i = 0;
		while (i < lengthOfWordPool) {
			if (i < letterAmount) {
				wordPool[i] = (char) ('A' + i);
			} else if (i < letterAmount * 2) {
				wordPool[i] = (char) ('a' + i - letterAmount);
			} else if (i < letterAmount * 2 + numberAmount) {
				wordPool[i] = (char) ('0' + i - letterAmount * 2);
			}
			i++;
		}
	}

	public static void genFile(String fileName) {
		try (Writer out = new FileWriter(fileName)) {
			String content = rap.genContent();
			out.write(content);
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private String genContent() {
		StringBuilder sBuilder = new StringBuilder();
		rap.feedContent(sBuilder);
		return sBuilder.toString();
	}

	private void feedContent(StringBuilder sBuilder) {
		int numberOfSentences = minSentencesPerFile + (int) (Math.random() *
		sentencesRange);

		for (int i = 0; i < numberOfSentences + 1; i++)
			rap.feedSentence(sBuilder);
	}

	private void feedSentence(StringBuilder sBuilder) {
		int numberOfWords = minWordsPerSentence + (int) (Math.random() *
		wordsRange);

		for (int i = 0; i < numberOfWords + 1; i++)
			rap.feedWord(sBuilder);
	}

	private void feedWord(StringBuilder sBuilder) {
		int numberOfChars = minCharsPerWord + (int) (Math.random() *
		charsRange);

		for (int i = 0; i < numberOfChars + 1; i++)
			sBuilder.append(wordPool[(int) (Math.random() * lengthOfWordPool)]);
		sBuilder.append(symbolPool[(int) (Math.random() * lengthOfSymbolPool)]);
	}
}