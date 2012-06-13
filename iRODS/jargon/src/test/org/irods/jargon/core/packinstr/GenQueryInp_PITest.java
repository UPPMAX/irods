package org.irods.jargon.core.packinstr;

import java.util.Properties;

import junit.framework.Assert;
import junit.framework.TestCase;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.query.IRODSQuery;
import org.irods.jargon.core.query.IRODSQueryTranslator;
import org.irods.jargon.core.query.RodsGenQueryEnum;
import org.irods.jargon.core.query.TranslatedIRODSQuery;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import edu.sdsc.jargon.testutils.TestingPropertiesHelper;

public class GenQueryInp_PITest {

	private static Properties testingProperties = new Properties();
	private static TestingPropertiesHelper testingPropertiesHelper = new TestingPropertiesHelper();

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		TestingPropertiesHelper testingPropertiesLoader = new TestingPropertiesHelper();
		testingProperties = testingPropertiesLoader.getTestProperties();
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testGenQueryInp_PI() throws Exception {
		String queryString = "select " 
			+ RodsGenQueryEnum.COL_D_COLL_ID.getName() 
			+ " ," 
			+ RodsGenQueryEnum.COL_COLL_ACCESS_COLL_ID.getName()
			+ " where "
			+ RodsGenQueryEnum.COL_COLL_ACCESS_TYPE.getName()
			+ " = "
			+ "'2'";
		
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2", "d", "zone");
		
		IRODSQuery irodsQuery = IRODSQuery.instance(queryString, 500);
		
		IRODSQueryTranslator irodsQueryTranslator = new IRODSQueryTranslator(props);
		TranslatedIRODSQuery translatedIRODSQuery = irodsQueryTranslator.getTranslatedQuery(irodsQuery);
		
		GenQueryInp genQueryInp = GenQueryInp.instance(translatedIRODSQuery, 0);
		Assert.assertNotNull(genQueryInp.getParsedTags());
	}

	

	@Test
	public final void testGetParsedTags() throws Exception {
		String queryString = "select " 
			+ RodsGenQueryEnum.COL_D_COLL_ID.getName() 
			+ " ," 
			+ RodsGenQueryEnum.COL_COLL_ACCESS_COLL_ID.getName()
			+ " where "
			+ RodsGenQueryEnum.COL_COLL_ACCESS_TYPE.getName()
			+ " = "
			+ "'2'";
		
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2", "d", "zone");
		
		IRODSQuery irodsQuery = IRODSQuery.instance(queryString, 500);
		IRODSQueryTranslator irodsQueryTranslator = new IRODSQueryTranslator(props);
		TranslatedIRODSQuery translatedIRODSQuery = irodsQueryTranslator.getTranslatedQuery(irodsQuery);
		GenQueryInp genQueryInp = GenQueryInp.instance(translatedIRODSQuery, 0);		
		String tagData = genQueryInp.getParsedTags();
		Assert.assertTrue("did not find select field", tagData.indexOf(String.valueOf(RodsGenQueryEnum.COL_D_COLL_ID.getNumericValue())) > -1);
		Assert.assertTrue("did not find select field", tagData.indexOf(String.valueOf(RodsGenQueryEnum.COL_COLL_ACCESS_COLL_ID.getNumericValue())) > -1);

	}

}
