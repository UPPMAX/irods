package edu.sdsc.jargon.testutils;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.GENERATED_FILE_DIRECTORY_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_HOST_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PASSWORD_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_PORT_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_RESOURCE_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_USER_KEY;
import static edu.sdsc.jargon.testutils.TestingPropertiesHelper.IRODS_ZONE_KEY;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import java.net.URI;
import java.net.URISyntaxException;

import java.util.Properties;

/**
 * Utilities to load testing properties from a properties file
 *
 * @author Mike Conway, DICE (www.irods.org)
 * @since 10/18/2009
 */
public class TestingPropertiesHelper {
	public static String GENERATED_FILE_DIRECTORY_KEY = "test.data.directory";
	public static String IRODS_USER_KEY = "test.irods.user";
	public static String IRODS_PASSWORD_KEY = "test.irods.password";
	public static String IRODS_RESOURCE_KEY = "test.irods.resource";
	public static String IRODS_USER_DN_KEY = "test.irods.userDN";
	public static String IRODS_SECONDARY_USER_KEY = "test2.irods.user";
	public static String IRODS_SECONDARY_PASSWORD_KEY = "test2.irods.password";
	public static String IRODS_SECONDARY_RESOURCE_KEY = "test2.irods.resource";
	public static String IRODS_RESOURCE_GROUP_KEY = "test.resource.group";
	public static String IRODS_HOST_KEY = "test.irods.host";
	public static String IRODS_PORT_KEY = "test.irods.port";
	public static String IRODS_ZONE_KEY = "test.irods.zone";
	public static String IRODS_SCRATCH_DIR_KEY = "test.irods.scratch.subdir";
	public static String IRODS_CONFIRM_TESTING_KEY = "test.confirm";
	public static String IRODS_ADMIN_USER_KEY = "test.irods.admin";
	public static String IRODS_ADMIN_PASSWORD_KEY = "test.irods.admin.password";
	public static String MAC_ICOMMANDS_PATH="test.mac.icommand.path";

	public static String IRODS_CONFIRM_TESTING_TRUE = "true";
	public static String IRODS_CONFIRM_TESTING_FALSE = "false";
	
	public int getPortAsInt(Properties testingProperties) throws TestingUtilsException {
		String portString = (String) testingProperties.get(IRODS_PORT_KEY);
		
		if (portString == null || portString.length() == 0) {
			throw new TestingUtilsException("missing or invalid test.irods.port in testing.properties");
		}
		
		int retVal = 0; 
		
		try {
			retVal = Integer.parseInt(portString);
		} catch (NumberFormatException nfe) {
			throw new TestingUtilsException("port is in valid format to convert to int:" + portString, nfe);
		}
		
		return retVal;
	}


	/**
	 * Load the properties that control various tests from the
	 * testing.properties file on the code path
	 *
	 * @return <code>Properties</code> class with the test values
	 * @throws TestingUtilsException
	 */
	public Properties getTestProperties() throws TestingUtilsException {
		ClassLoader loader = this.getClass().getClassLoader();
		InputStream in = loader.getResourceAsStream("testing.properties");
		Properties properties = new Properties();

		try {
			properties.load(in);
		} catch (IOException ioe) {
			throw new TestingUtilsException("error loading test properties",
					ioe);
		} finally {
			try {
				in.close();
			} catch (Exception e) {
				// ignore
			}
		}

		String testingConfirm = properties.getProperty(IRODS_CONFIRM_TESTING_KEY);
		if (testingConfirm == null) {
			throw new TestingUtilsException("did not find a test.confirm property in testing.properties");
		} else if (!testingConfirm.equals(IRODS_CONFIRM_TESTING_TRUE)) {
			throw new TestingUtilsException("test.confirm property in testing.properties needs to be set to true");
		}

		return properties;
	}



	/**
	 * Get a URI in IRODS format that points to a scratch file, given the file name and any additional path
	 * to that file without a leading '/'.  For example:
	 *
	 * Given that I have a file under the irods collection /test1/home/test/test-scratch/an_irods_subdir/file.txt
	 *
	 * I can construct a proper URI like this:
	 * <pre>
	 *
	 *
	 	StringBuilder uriPath = new StringBuilder();
        	uriPath.append("an_irods_subdir");
        	uriPath.append('/');
        	uriPath.append(file.txt);


        URI irodsUri = testingPropertiesHelper.buildUriFromTestPropertiesForFileInUserDir(testingProperties,
                uriPath.toString());
	 *
	 *
	 * </pre>
	 *
	 * Note that the scratch directory and everything above it is computed from testing.properties
	 *
	 * @param testingProperties
	 *            <code>Properties</code> file with the standard names defined
	 *            in
	 *            {@link org.TestingPropertiesHelper.jargon.test.utils.TestingPropertiesLoader}
	 * @param fileName
	 *            <code>String</code> with the path (no leading '/') below the
	 *            user scratch directory as defined in testing.properties
	 * @return <code>URI</code>
	 * @throws URISyntaxException
	 */
	public URI buildUriFromTestPropertiesForFileInUserDir(
			Properties testingProperties, String fileName)
			throws URISyntaxException {
		StringBuilder irodsUri = new StringBuilder();

		irodsUri.append("irods://");
		irodsUri.append(testingProperties.getProperty(IRODS_USER_KEY));
		irodsUri.append(".");
		irodsUri.append(testingProperties.getProperty(IRODS_ZONE_KEY));
		irodsUri.append(":");
		irodsUri.append(testingProperties.getProperty(IRODS_PASSWORD_KEY));
		irodsUri.append("@");
		irodsUri.append(testingProperties.getProperty(IRODS_HOST_KEY));
		irodsUri.append(":");
		irodsUri.append(String.valueOf(testingProperties
				.getProperty(IRODS_PORT_KEY)));
		irodsUri.append("/");
		irodsUri.append(testingProperties.get(IRODS_ZONE_KEY));
		irodsUri.append("/home/");
		irodsUri.append(testingProperties.get(IRODS_USER_KEY));
		irodsUri.append("/");
		irodsUri.append(testingProperties.getProperty(IRODS_SCRATCH_DIR_KEY));
		irodsUri.append("/");
		irodsUri.append(fileName);

		return new URI(irodsUri.toString());
	}
	
	/**
	 * Create a URI compatable with a local <code>IRODSFile</code> based on a set of peroperties, as well as
	 * a relative path (no leading '/') underneath the users local scratch directory as defined in testing.properties.
	 * @param testingProperties <code>Properties</code> defined in the testing.properties file
	 * @param fileName <code>String</code> containing the relative path (no leading '/') underneath
	 * the local scratch directory.  Note that the subdirectories will be created if they do not aleady exist.
	 * @return <code>URI</code> to a local file in a format that can be used with an <code>IRODSFile</code>
	 * @throws URISyntaxException
	 */
	public URI buildUriFromTestPropertiesForFileInLocalScratchDir(
			Properties testingProperties, String fileName)
			throws URISyntaxException {
		StringBuilder irodsUri = new StringBuilder();

		irodsUri.append("file://");
		File scratchDir = new File(testingProperties
				.getProperty(GENERATED_FILE_DIRECTORY_KEY)
				+ fileName);
		scratchDir.mkdirs();
		irodsUri.append(scratchDir.getAbsolutePath());

		return new URI(irodsUri.toString());
	}

	/**
	 * Get a URI in IRODS format that points to a scratch file, given the file name and any additional path
	 * to that file without a leading '/', for the secondary testing user.  For example:
	 *
	 * Note that the scratch directory and everything above it is computed from testing.properties
	 *
	 * @param testingProperties
	 *            <code>Properties</code> file with the standard names defined
	 *            in
	 *            {@link org.TestingPropertiesHelper.jargon.test.utils.TestingPropertiesLoader}
	 * @param fileName
	 *            <code>String</code> with the path (no leading '/') below the
	 *            user scratch directory as defined in testing.properties
	 * @return <code>URI</code>
	 * @throws URISyntaxException
	 */
	public URI buildUriFromTestPropertiesForFileInSecondaryUserDir(
			Properties testingProperties, String fileName)
			throws URISyntaxException {
		StringBuilder irodsUri = new StringBuilder();

		irodsUri.append("irods://");
		irodsUri.append(testingProperties.getProperty(IRODS_SECONDARY_USER_KEY));
		irodsUri.append(".");
		irodsUri.append(testingProperties.getProperty(IRODS_ZONE_KEY));
		irodsUri.append(":");
		irodsUri.append(testingProperties.getProperty(IRODS_SECONDARY_PASSWORD_KEY));
		irodsUri.append("@");
		irodsUri.append(testingProperties.getProperty(IRODS_HOST_KEY));
		irodsUri.append(":");
		irodsUri.append(String.valueOf(testingProperties
				.getProperty(IRODS_PORT_KEY)));
		irodsUri.append("/");
		irodsUri.append(testingProperties.get(IRODS_ZONE_KEY));
		irodsUri.append("/home/");
		irodsUri.append(testingProperties.get(IRODS_SECONDARY_USER_KEY));
		irodsUri.append("/");
		irodsUri.append(testingProperties.getProperty(IRODS_SCRATCH_DIR_KEY));
		irodsUri.append("/");
		irodsUri.append(fileName);

		return new URI(irodsUri.toString());
	}

	/**
	 * @param testingProperties
	 *            <code>Properties</code> file with the standard names defined
	 *            in
	 *            {@link org.TestingPropertiesHelper.jargon.test.utils.TestingPropertiesLoader}
	 * @return @link{ edu.sdsc.grid.io.irods.IRODSAccount}
	 * @throws URISyntaxException
	 */
	public IRODSAccount buildIRODSAdminAccountFromTestProperties(
			Properties testingProperties)  {

		 StringBuilder homeBuilder = new StringBuilder();
	        homeBuilder.append('/');
	        homeBuilder.append(testingProperties.getProperty(IRODS_ZONE_KEY));
	        homeBuilder.append('/');
	        homeBuilder.append("home");
	        homeBuilder.append('/');
	        homeBuilder.append(testingProperties.getProperty(IRODS_USER_KEY));

		IRODSAccount account = new IRODSAccount(
				testingProperties.getProperty(IRODS_HOST_KEY),
				Integer.parseInt(testingProperties.getProperty(IRODS_PORT_KEY)),
				testingProperties.getProperty(IRODS_ADMIN_USER_KEY),
				testingProperties.getProperty(IRODS_ADMIN_PASSWORD_KEY),
				homeBuilder.toString(),
				testingProperties.getProperty(IRODS_ZONE_KEY),
				testingProperties.getProperty(IRODS_RESOURCE_KEY));

		return account;
	}

	/**
	 * @param testingProperties
	 *            <code>Properties</code> file with the standard names defined
	 *            in
	 *            {@link org.TestingPropertiesHelper.jargon.test.utils.TestingPropertiesLoader}
	 * @return @link{ edu.sdsc.grid.io.irods.IRODSAccount}
	 * @throws URISyntaxException
	 */
	public IRODSAccount buildIRODSAccountFromTestProperties(
			Properties testingProperties)  {

		 StringBuilder homeBuilder = new StringBuilder();
	        homeBuilder.append('/');
	        homeBuilder.append(testingProperties.getProperty(IRODS_ZONE_KEY));
	        homeBuilder.append('/');
	        homeBuilder.append("home");
	        homeBuilder.append('/');
	        homeBuilder.append(testingProperties.getProperty(IRODS_USER_KEY));

		IRODSAccount account = new IRODSAccount(
				testingProperties.getProperty(IRODS_HOST_KEY),
				Integer.parseInt(testingProperties.getProperty(IRODS_PORT_KEY)),
				testingProperties.getProperty(IRODS_USER_KEY),
				testingProperties.getProperty(IRODS_PASSWORD_KEY),
				homeBuilder.toString(),
				testingProperties.getProperty(IRODS_ZONE_KEY),
				testingProperties.getProperty(IRODS_RESOURCE_KEY));

		return account;
	}

	/**
	 * @param testingProperties
	 *            <code>Properties</code> file with the standard names defined
	 *            in
	 *            {@link org.TestingPropertiesHelper.jargon.test.utils.TestingPropertiesLoader}
	 * @return @link{ edu.sdsc.grid.io.irods.IRODSAccount}
	 * @throws URISyntaxException
	 */
	public IRODSAccount buildIRODSAccountFromSecondaryTestProperties(
			Properties testingProperties)  {

		 StringBuilder homeBuilder = new StringBuilder();
	        homeBuilder.append('/');
	        homeBuilder.append(testingProperties.getProperty(IRODS_ZONE_KEY));
	        homeBuilder.append('/');
	        homeBuilder.append("home");
	        homeBuilder.append('/');
	        homeBuilder.append(testingProperties.getProperty(IRODS_SECONDARY_USER_KEY));

		IRODSAccount account = new IRODSAccount(
				testingProperties.getProperty(IRODS_HOST_KEY),
				Integer.parseInt(testingProperties.getProperty(IRODS_PORT_KEY)),
				testingProperties.getProperty(IRODS_SECONDARY_USER_KEY),
				testingProperties.getProperty(IRODS_SECONDARY_PASSWORD_KEY),
				homeBuilder.toString(),
				testingProperties.getProperty(IRODS_ZONE_KEY),
				testingProperties.getProperty(IRODS_SECONDARY_RESOURCE_KEY));

		return account;
	}

	/**
	 * @param testingProperties
	 *            <code>Properties</code> file with the standard names defined
	 *            in
	 *            {@link org.TestingPropertiesHelper.jargon.test.utils.TestingPropertiesLoader}
	 * @return @link{
	 *         edu.sdsc.jargon.testutils.icommandinvoker.IrodsInvocationContext}
	 * @throws URISyntaxException
	 */
	public IrodsInvocationContext buildIRODSInvocationContextFromTestProperties(
			Properties testingProperties) {

		IrodsInvocationContext context = new IrodsInvocationContext();
		context.setIrodsHost(testingProperties.getProperty(IRODS_HOST_KEY));
		context.setIrodsPassword(testingProperties
				.getProperty(IRODS_PASSWORD_KEY));
		context.setIrodsPort(Integer.parseInt(testingProperties
				.getProperty(IRODS_PORT_KEY)));
		context.setIrodsUser(testingProperties.getProperty(IRODS_USER_KEY));
		context.setIrodsZone(testingProperties.getProperty(IRODS_ZONE_KEY));
		context.setLocalWorkingDirectory(testingProperties
				.getProperty(GENERATED_FILE_DIRECTORY_KEY));
		context.setIrodsScratchDir(testingProperties
				.getProperty(IRODS_SCRATCH_DIR_KEY));
		context.setIrodsResource(testingProperties
				.getProperty(IRODS_RESOURCE_KEY));

		return context;
	}

	/**
	 * @param testingProperties
	 *            <code>Properties</code> file with the standard names defined
	 *            in
	 *            {@link org.TestingPropertiesHelper.jargon.test.utils.TestingPropertiesLoader}
	 * @return @link{
	 *         edu.sdsc.jargon.testutils.icommandinvoker.IrodsInvocationContext}
	 * @throws URISyntaxException
	 */
	public IrodsInvocationContext buildIRODSInvocationContextFromSecondaryTestProperties(
			Properties testingProperties) {

		IrodsInvocationContext context = new IrodsInvocationContext();
		context.setIrodsHost(testingProperties.getProperty(IRODS_HOST_KEY));
		context.setIrodsPassword(testingProperties
				.getProperty(IRODS_SECONDARY_PASSWORD_KEY));
		context.setIrodsPort(Integer.parseInt(testingProperties
				.getProperty(IRODS_PORT_KEY)));
		context.setIrodsUser(testingProperties.getProperty(IRODS_SECONDARY_USER_KEY));
		context.setIrodsZone(testingProperties.getProperty(IRODS_ZONE_KEY));
		context.setLocalWorkingDirectory(testingProperties
				.getProperty(GENERATED_FILE_DIRECTORY_KEY));
		context.setIrodsScratchDir(testingProperties
				.getProperty(IRODS_SCRATCH_DIR_KEY));
		context.setIrodsResource(testingProperties
				.getProperty(IRODS_SECONDARY_RESOURCE_KEY));

		return context;
	}

	/**
	 * Handy method to give, from the root IRODS collection, a full path to a
	 * given collection in the IRODS test scratch area on IRODS
	 *
	 * @param testingProperties
	 *            <code>Properties</code> that define test behavior
	 * @param collectionPathBelowScratch
	 *            <code>String</code> with no leading '/' that defines the
	 *            desired path underneath the IRODS scratch directory
	 * @return <code>String</code> with trailing '/' that gives the absolute
	 *         path for an IRODS collection
	 * @throws TestingUtilsException
	 * @throws URISyntaxException
	 */
	public String buildIRODSCollectionAbsolutePathFromTestProperties(
			Properties testingProperties, String collectionPathBelowScratch) throws TestingUtilsException {

		if (testingProperties.get(IRODS_SCRATCH_DIR_KEY) == null) {
			throw new TestingUtilsException("scratch path not provided in testing.properties");
		}

		StringBuilder pathBuilder = new StringBuilder();
		pathBuilder.append('/');
		pathBuilder.append(testingProperties.get(IRODS_ZONE_KEY));
		pathBuilder.append("/home/");
		pathBuilder.append(testingProperties.get(IRODS_USER_KEY));
		pathBuilder.append('/');
		pathBuilder.append(testingProperties.get(IRODS_SCRATCH_DIR_KEY));
		pathBuilder.append('/');
		pathBuilder.append(collectionPathBelowScratch);
		return pathBuilder.toString();
	}

	/**
	 * Handy method to give, from the root IRODS collection, a full path to a
	 * given collection in the IRODS test scratch area on IRODS
	 *
	 * @param testingProperties
	 *            <code>Properties</code> that define test behavior
	 * @param collectionPathBelowScratch
	 *            <code>String</code> with no leading '/' that defines the
	 *            desired path underneath the IRODS scratch directory
	 * @return <code>String</code> with trailing '/' that gives the absolute
	 *         path for an IRODS collection
	 * @throws TestingUtilsException
	 * @throws URISyntaxException
	 */
	public String buildIRODSCollectionAbsolutePathFromSecondaryTestProperties(
			Properties testingProperties, String collectionPathBelowScratch) throws TestingUtilsException {

		if (testingProperties.get(IRODS_SCRATCH_DIR_KEY) == null) {
			throw new TestingUtilsException("scratch path not provided in testing.properties");
		}

		StringBuilder pathBuilder = new StringBuilder();
		pathBuilder.append('/');
		pathBuilder.append(testingProperties.get(IRODS_ZONE_KEY));
		pathBuilder.append("/home/");
		pathBuilder.append(testingProperties.get(IRODS_SECONDARY_USER_KEY));
		pathBuilder.append('/');
		pathBuilder.append(testingProperties.get(IRODS_SCRATCH_DIR_KEY));
		pathBuilder.append('/');
		pathBuilder.append(collectionPathBelowScratch);
		return pathBuilder.toString();
	}


	/**
	 * Handy method to give, from the root IRODS collection, a relative path
	 * under the home directory for the described user
	 *
	 * @param testingProperties
	 *            <code>Properties</code> that define test behavior
	 * @param collectionPathBelowScratch
	 *            <code>String</code> with no leading '/' that defines the
	 *            desired path underneath the IRODS scratch directory
	 * @return <code>String</code> with trailing '/' that gives the absolute
	 *         path for an IRODS collection
	 * @throws URISyntaxException
	 */
	public String buildIRODSCollectionRelativePathFromTestProperties(
			Properties testingProperties, String collectionPathBelowScratch) {
		StringBuilder pathBuilder = new StringBuilder();
		pathBuilder.append(testingProperties.get(IRODS_SCRATCH_DIR_KEY));
		pathBuilder.append('/');
		pathBuilder.append(collectionPathBelowScratch);
		return pathBuilder.toString();
	}
}
