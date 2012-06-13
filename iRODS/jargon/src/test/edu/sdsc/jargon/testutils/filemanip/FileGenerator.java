/**
 *
 */
package edu.sdsc.jargon.testutils.filemanip;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.Random;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.TestingUtilsException;

/**
 * Helper methods to generate dummy files and directories useful for Jargon
 * testing
 * 
 * @author Mike Conway, DICE (www.irods.org)
 * @since 10/16/2009
 * 
 */
public class FileGenerator {

	public static List<String> fileExtensions = new ArrayList<String>();
	private static final Random RANDOM = new Random();
	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();

	static {
		fileExtensions.add(".doc");
		fileExtensions.add(".txt");
		fileExtensions.add(".csv");
		fileExtensions.add(".gif");
		fileExtensions.add(".jpg");
		fileExtensions.add(".avi");
		try {
			testingProperties = testingPropertiesHelper.getTestProperties();
		} catch (TestingUtilsException e) {
			throw new IllegalStateException("cannot find testing properties", e);
		}
	}

	/**
	 * Create a random file extension selected from the
	 * <code>fileExtensions</code> options
	 * 
	 * @return <code>String</code> containing a random, though valid, file
	 *         extension prepended with a '.' character
	 * @throws TestingUtilsException
	 */
	protected static String generateRandomExtension()
			throws TestingUtilsException {
		return fileExtensions
				.get(generateRandomNumber(0, fileExtensions.size()));
	}

	/**
	 * Generate a random string
	 * 
	 * @param length
	 *            <code>int</code> that determines the length of the
	 *            <code>String</code> that is generated.
	 * @return <code>String</code> of n length composed of random alphabetic
	 *         characters
	 */
	public static String generateRandomString(int length) {

		StringBuilder outString = new StringBuilder();
		Random generator = new Random();
		char theChar = 0;
		int outLength = 0;

		while (outLength < length) {
			theChar = (char) (generator.nextInt(65) + 65);
			if (Character.isLetter(theChar)) {
				outString.append(theChar);
				outLength++;
			}

		}
		return outString.toString();
	}

	/**
	 * Generate a random file name + extension
	 * 
	 * @param length
	 *            <code>int</code> that determines the length of the file name
	 *            (not including the extension this will be appended)
	 * @return <code>String</code> which is a random file name plus a random
	 *         extension
	 * @throws TestingUtilsException
	 */
	protected static String generateRandomFileName(int length)
			throws TestingUtilsException {
		StringBuilder fileName = new StringBuilder();
		fileName.append(generateRandomString(length));
		fileName.append(generateRandomExtension());
		return fileName.toString();
	}
	
	/**
	 * Given a directory, generate a randomly named file with random data of
	 * provided length.
	 * 
	 * @param fileDirectory
	 *            <code>String</code> containing a an absolute path to valid directory on the local
	 *            file system
	 * @param length
	 *            <code>long</code> containing the desired length of the file in bytes
	 * @return <code>String</code> containing the full path to the generated
	 *         file
	 * @throws TestingUtilsException
	 */
	public static String generateFileOfFixedLengthGivenName(String fileDirectory, String fileName,
			long length) throws TestingUtilsException {
		
		// 1024 bytes of random stuff should be plenty, then just repeat it as needed
		
		long chunkSize = 1024;
		if (length <= chunkSize) {
			chunkSize = (int) length;
		}

		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		int generatedLength = 0;
		
		while (generatedLength < chunkSize) {  
			bos.write(RANDOM.nextInt());
			generatedLength += 1;
		}
		
		byte[] fileChunk = bos.toByteArray();
		
		// take the chunk and fill up the file
		
		File randFile = new File(fileDirectory, fileName);
		OutputStream outStream = null;
		
		long generatedFileLength = 0;
		long nextChunkSize = 0;
		try {
			outStream = new BufferedOutputStream(new FileOutputStream(randFile));

			while (generatedFileLength < length) {
				
				// if more than chunk size to go, just write the chunk
				nextChunkSize = length - generatedFileLength;
				if (nextChunkSize > chunkSize) {
					outStream.write(fileChunk);
					generatedFileLength += chunkSize;
				} else {
					outStream.write(fileChunk, 0, (int) nextChunkSize);
					generatedFileLength += nextChunkSize;
				}
				
			}

		} catch (IOException ioe) {
			throw new TestingUtilsException(
					"error generating random file with dir:" + fileDirectory
							+ " and generated name:" + fileName, ioe);
		} finally {
			if (outStream != null) {
				try {

					outStream.close();
				} catch (Exception ex) {
					// ignore
				}
			}
		}
		
		StringBuilder fullPath = new StringBuilder();
		fullPath.append(fileDirectory);
		fullPath.append(fileName);
		return fullPath.toString();

	}

	public static int generateRandomNumber(int min, int max)
			throws TestingUtilsException {
		if (max < min) {
			throw new TestingUtilsException(
					"max length must be > or = min length");
		}

		int range = max - min;
		Random generator = new Random();
		return min + generator.nextInt(range);
	}

	public static List<String>  generateManyFilesInGivenDirectory(String relativePathUnderScratch, String filePrefix, String fileSuffix,
			int numberOfFiles, int fileLengthMin, int fileLengthMax) throws TestingUtilsException {
		// n number of random files in the source directory, with a random
		// length between the min and max
		
		ScratchFileUtils scratchFileUtils = new ScratchFileUtils(testingProperties);
		String absPath = scratchFileUtils
		.createAndReturnAbsoluteScratchPath(relativePathUnderScratch);
		ArrayList<String> sourceFileNames = new ArrayList<String>();
		String genFileName = "";
		for (int i = 0; i < numberOfFiles; i++) {
			genFileName = filePrefix + i + fileSuffix;
			genFileName = FileGenerator.generateFileOfFixedLengthGivenName(
					absPath, genFileName, FileGenerator.generateRandomNumber(
							fileLengthMin, fileLengthMax));
			sourceFileNames.add(genFileName);
		}
		
		return sourceFileNames;		
	}
	
	
}
