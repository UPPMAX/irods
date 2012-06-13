package org.irods.jargon.core.accessobject;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Properties;

import junit.framework.Assert;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.grid.io.irods.IRODSAccount;
import edu.sdsc.grid.io.irods.IRODSFileSystem;
import edu.sdsc.jargon.testutils.IRODSTestSetupUtilities;
import edu.sdsc.jargon.testutils.TestingPropertiesHelper;
import edu.sdsc.jargon.testutils.filemanip.FileGenerator;
import edu.sdsc.jargon.testutils.filemanip.ScratchFileUtils;
import edu.sdsc.jargon.testutils.icommandinvoke.IcommandInvoker;
import edu.sdsc.jargon.testutils.icommandinvoke.IrodsInvocationContext;
import edu.sdsc.jargon.testutils.icommandinvoke.icommands.IputCommand;

public class RemoteExecutionOfCommandsAOImplTest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();
	private static ScratchFileUtils scratchFileUtils = null;
	public static final String IRODS_TEST_SUBDIR_PATH = "RemoteExecutionOfCommandsAOImplTest";
	private static IRODSTestSetupUtilities irodsTestSetupUtilities = null;

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
		scratchFileUtils = new ScratchFileUtils(testingProperties);
		scratchFileUtils
				.clearAndReinitializeScratchDirectory(IRODS_TEST_SUBDIR_PATH);
		irodsTestSetupUtilities = new IRODSTestSetupUtilities();
		irodsTestSetupUtilities.initializeIrodsScratchDirectory();
		irodsTestSetupUtilities
				.initializeDirectoryForTest(IRODS_TEST_SUBDIR_PATH);
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testRemoteExecutionOfCommandsAOImpl() throws Exception {
		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		RemoteExecutionOfCommandsAO remoteExecutionOfCommandsAO = accessObjectFactory.getRemoteExecutionOfCommandsAO();
		Assert.assertNotNull("no remote commands executer found",
				remoteExecutionOfCommandsAO);
	}

	@Test
	public final void testExecuteARemoteCommandAndGetStreamGivingCommandNameAndArgs()
			throws Exception {
		String cmd = "hello";
		String args = "";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		RemoteExecutionOfCommandsAO remoteExecutionOfCommandsAO = accessObjectFactory.getRemoteExecutionOfCommandsAO();

		InputStream inputStream = remoteExecutionOfCommandsAO
				.executeARemoteCommandAndGetStreamGivingCommandNameAndArgs(cmd,
						args);

		BufferedReader br = new BufferedReader(new InputStreamReader(
				inputStream));
		StringBuilder sb = new StringBuilder();
		String line = null;

		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}

		br.close();
		String result = sb.toString();
		irodsFileSystem.close();

		Assert.assertEquals("did not successfully execute hello command",
				"Hello world  from irods".trim(), result.trim());
	}

	@Test
	public final void testExecuteARemoteCommandAndGetStreamGivingCommandNameAndArgsAndHost()
			throws Exception {
		String cmd = "hello";
		String args = "";
		String host = "localhost";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);
		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		RemoteExecutionOfCommandsAO remoteExecutionOfCommandsAO = accessObjectFactory.getRemoteExecutionOfCommandsAO();

		InputStream inputStream = remoteExecutionOfCommandsAO
				.executeARemoteCommandAndGetStreamGivingCommandNameAndArgsAndHost(
						cmd, args, host);
		BufferedReader br = new BufferedReader(new InputStreamReader(
				inputStream));
		StringBuilder sb = new StringBuilder();
		String line = null;

		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}

		br.close();
		String result = sb.toString();
		irodsFileSystem.close();

		Assert.assertEquals("did not successfully execute hello command",
				"Hello world  from irods".trim(), result.trim());
	}

	@Test
	public final void testExecuteARemoteCommandAndGetStreamUsingAnIRODSFileAbsPathToDetermineHost()
			throws Exception {
		String cmd = "hello";
		String args = "";

		IRODSAccount irodsAccount = testingPropertiesHelper
				.buildIRODSAccountFromTestProperties(testingProperties);

		String testFileName = "testExecuteARemoteCommandAndGetStreamUsingAnIRODSFileAbsPathToDetermineHost.txt";
		String absPath = scratchFileUtils
				.createAndReturnAbsoluteScratchPath(IRODS_TEST_SUBDIR_PATH);
		String localFileName = FileGenerator
				.generateFileOfFixedLengthGivenName(absPath, testFileName, 1);

		String targetIrodsFile = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH + '/'
								+ testFileName);

		IrodsInvocationContext invocationContext = testingPropertiesHelper
				.buildIRODSInvocationContextFromTestProperties(testingProperties);
		IputCommand iputCommand = new IputCommand();

		String targetIrodsCollection = testingPropertiesHelper
				.buildIRODSCollectionAbsolutePathFromTestProperties(
						testingProperties, IRODS_TEST_SUBDIR_PATH);

		iputCommand.setLocalFileName(localFileName);
		iputCommand.setIrodsFileName(targetIrodsCollection);
		iputCommand.setForceOverride(true);

		IcommandInvoker invoker = new IcommandInvoker(invocationContext);
		invoker.invokeCommandAndGetResultAsString(iputCommand);

		IRODSFileSystem irodsFileSystem = new IRODSFileSystem(irodsAccount);

		IRODSAccessObjectFactory accessObjectFactory = IRODSAccessObjectFactoryImpl
				.instance(irodsFileSystem.getCommands());
		RemoteExecutionOfCommandsAO remoteExecutionOfCommandsAO = accessObjectFactory.getRemoteExecutionOfCommandsAO();

		InputStream inputStream = remoteExecutionOfCommandsAO
				.executeARemoteCommandAndGetStreamUsingAnIRODSFileAbsPathToDetermineHost(
						cmd, args, targetIrodsFile);

		BufferedReader br = new BufferedReader(new InputStreamReader(
				inputStream));
		StringBuilder sb = new StringBuilder();
		String line = null;

		while ((line = br.readLine()) != null) {
			sb.append(line + "\n");
		}

		br.close();
		String result = sb.toString();
		irodsFileSystem.close();

		Assert.assertEquals("did not successfully execute hello command",
				"Hello world  from irods".trim(), result.trim());
	}

}
