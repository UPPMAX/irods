package org.irods.jargon.core.query;

import java.util.List;

import junit.framework.TestCase;

import org.irods.jargon.core.connection.IRODSServerProperties;
import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class IRODSQueryTranslatorTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testIRODSQueryTranslator() throws Exception {
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.3",
				"d", "zone");
		new IRODSQueryTranslator(props);

	}

	@Test(expected = JargonException.class)
	public final void testIRODSQueryTranslatorNullServerProps()
			throws Exception {
		// test passes if no exceptions were thrown
		IRODSQueryTranslator translator = new IRODSQueryTranslator(null);
	}

	@Test
	public final void testParseSelectsIntoListOfNames() throws Exception {
		String query = "select blah, yelp";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		List<String> selects = translator.parseSelectsIntoListOfNames(query);
		TestCase.assertEquals(2, selects.size());
		TestCase.assertEquals("blah", selects.get(0));
		TestCase.assertEquals("yelp", selects.get(1));
	}

	@Test(expected = JargonQueryException.class)
	public final void testParseSelectsIntoListOfNamesNoSelect()
			throws Exception {
		String query = "blah, yelp";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		translator.parseSelectsIntoListOfNames(query);

	}

	@Test
	public final void testParseConditionsIntoList() throws Exception {
		String query = "select blah, yelp where a = 1";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		List<QueryCondition> conditions = translator
				.parseConditionsIntoList(query);
		TestCase.assertEquals(1, conditions.size());
		QueryCondition cond = conditions.get(0);
		TestCase.assertEquals("a", cond.getFieldName());
		TestCase.assertEquals("=", cond.getOperator());
		TestCase.assertEquals("1", cond.getValue());

	}

	@Test(expected = JargonQueryException.class)
	public final void testParseConditionsIntoListNoSpaceAfterOperatorFinalPosition()
			throws Exception {
		String query = "select blah, yelp where a =1";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		translator.parseConditionsIntoList(query);

	}

	@Test(expected = JargonQueryException.class)
	public final void testParseConditionsIntoListNoSpaceAfterOperator()
			throws Exception {
		String query = "select blah, yelp where a =1 and x = 4";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		translator.parseConditionsIntoList(query);

	}

	@Test
	public final void testParseTwoConditionsIntoList() throws Exception {
		String query = "select blah, yelp where a = 1 and z > 1234";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		List<QueryCondition> conditions = translator
				.parseConditionsIntoList(query);
		TestCase.assertEquals(2, conditions.size());
		QueryCondition cond = conditions.get(0);
		TestCase.assertEquals("a", cond.getFieldName());
		TestCase.assertEquals("=", cond.getOperator());
		TestCase.assertEquals("1", cond.getValue());

		cond = conditions.get(1);
		TestCase.assertEquals("z", cond.getFieldName());
		TestCase.assertEquals(">", cond.getOperator());
		TestCase.assertEquals("1234", cond.getValue());

	}

	@Test
	public final void testParseNoConditionsIntoList() throws Exception {
		String query = "select blah, yelp";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		List<QueryCondition> conditions = translator
				.parseConditionsIntoList(query);
		TestCase.assertEquals(0, conditions.size());
	}

	@Test(expected = JargonQueryException.class)
	public final void testConditions2WheresIntoList() throws Exception {
		String query = "select blah, yelp where where a = 2";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		translator.parseConditionsIntoList(query);
	}

	@Test(expected = JargonQueryException.class)
	public final void testConditionsIncompleteList() throws Exception {
		String query = "select blah, yelp where where a = ";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);
		translator.parseConditionsIntoList(query);
	}

	@Test
	public final void testTranslateOnlySelectsIntoTranslatedQuery()
			throws Exception {

		String query = ("select "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + "," + RodsGenQueryEnum.COL_AUDIT_USER_ID
				.getName());
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(2, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals(RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID
				.getName(), sel1.getSelectFieldColumnName());
		SelectField sel2 = translatedQuery.getSelectFields().get(1);
		TestCase.assertEquals(RodsGenQueryEnum.COL_AUDIT_USER_ID.getName(),
				sel2.getSelectFieldColumnName());
	}

	@Test
	public final void testDistinctQuery() throws Exception {

		String query = ("select "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + "," + RodsGenQueryEnum.COL_AUDIT_USER_ID
				.getName());
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertTrue("this should be classified as a distinct query",
				translatedQuery.isDistinct());

	}

	@Test
	public final void testNonDistinctQuery() throws Exception {

		String query = ("select non-distinct "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + "," + RodsGenQueryEnum.COL_AUDIT_USER_ID
				.getName());
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertFalse(
				"this should not be classified as a distinct query",
				translatedQuery.isDistinct());

	}

	@Test(expected = JargonQueryException.class)
	public final void testNonDistinctQuerySelectMissing() throws Exception {

		String query = ("non-distinct"
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + "," + RodsGenQueryEnum.COL_AUDIT_USER_ID
				.getName());
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		translator.getTranslatedQuery(irodsQuery);

	}

	@Test
	public final void testNonDistinctQueryUpperCase() throws Exception {

		String query = ("select NON-DISTINCT "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + "," + RodsGenQueryEnum.COL_AUDIT_USER_ID
				.getName());
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertFalse(
				"this should not be classified as a distinct query",
				translatedQuery.isDistinct());

	}

	@Test
	public final void testTranslateCountAggregation() throws Exception {

		String query = ("select count("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ")");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals("field not translated",
				RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName(), sel1
						.getSelectFieldColumnName());
		TestCase.assertEquals("did not classify as a count()",
				SelectField.SelectFieldTypes.COUNT, sel1.getSelectFieldType());

	}

	@Test
	public final void testTranslateSumAggregation() throws Exception {

		String query = ("select SUM("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ")");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals("field not translated",
				RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName(), sel1
						.getSelectFieldColumnName());
		TestCase.assertEquals("did not classify as a count()",
				SelectField.SelectFieldTypes.SUM, sel1.getSelectFieldType());

	}

	@Test
	public final void testTranslateAvgAggregation() throws Exception {

		String query = ("select Avg("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ")");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals("field not translated",
				RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName(), sel1
						.getSelectFieldColumnName());
		TestCase.assertEquals("did not classify as a count()",
				SelectField.SelectFieldTypes.AVG, sel1.getSelectFieldType());

	}

	@Test
	public final void testTranslateMinAggregation() throws Exception {

		String query = ("select mIn("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ")");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals("field not translated",
				RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName(), sel1
						.getSelectFieldColumnName());
		TestCase.assertEquals("did not classify as a count()",
				SelectField.SelectFieldTypes.MIN, sel1.getSelectFieldType());

	}

	@Test
	public final void testTranslateMaxAggregation() throws Exception {

		String query = ("select maX("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ")");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals("field not translated",
				RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName(), sel1
						.getSelectFieldColumnName());
		TestCase.assertEquals("did not classify as a count()",
				SelectField.SelectFieldTypes.MAX, sel1.getSelectFieldType());

	}

	@Test(expected = JargonQueryException.class)
	public final void testTranslateAggregationWithEmbeddedSpaces()
			throws Exception {

		String query = ("select mIn(  "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + "     )");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		translator.getTranslatedQuery(irodsQuery);

	}

	@Test(expected = JargonQueryException.class)
	public final void testTranslateCountAggregationOpenNoClose()
			throws Exception {

		String query = ("select count(" + RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID
				.getName());
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);

	}

	@Test(expected = JargonQueryException.class)
	public final void testTranslateAggregationInvalidAggregationType()
			throws Exception {

		String query = ("select bob("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ")");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);

	}

	@Test(expected = JargonQueryException.class)
	public final void testTranslateAggregationTwoOpenParens() throws Exception {

		String query = ("select SUM(("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ")");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertNull("should have null indicating irods lookup failed",
				sel1);

	}

	@Test
	public final void testTranslateAggregationTwoCloseParens() throws Exception {

		String query = ("select SUM("
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + "))");
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(1, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals("field not translated",
				RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName(), sel1
						.getSelectFieldColumnName());
		TestCase.assertEquals("did not classify as a count()",
				SelectField.SelectFieldTypes.SUM, sel1.getSelectFieldType());

	}

	@Test
	public final void testTranslateSelectsAndConditionsIntoTranslatedQuery()
			throws Exception {
		String query = ("select "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ","
				+ RodsGenQueryEnum.COL_AUDIT_USER_ID.getName() + " where "
				+ RodsGenQueryEnum.COL_AUDIT_OBJ_ID.getName() + " = '123'");

		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
		TestCase.assertEquals(2, translatedQuery.getSelectFields().size());
		SelectField sel1 = translatedQuery.getSelectFields().get(0);
		TestCase.assertEquals(RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID
				.getName(), sel1.getSelectFieldColumnName());
		SelectField sel2 = translatedQuery.getSelectFields().get(1);
		TestCase.assertEquals(RodsGenQueryEnum.COL_AUDIT_USER_ID.getName(),
				sel2.getSelectFieldColumnName());
		TestCase.assertEquals(1, translatedQuery.getTranslatedQueryConditions()
				.size());
		TranslatedQueryCondition testCondition = translatedQuery
				.getTranslatedQueryConditions().get(0);
		TestCase.assertEquals(RodsGenQueryEnum.COL_AUDIT_OBJ_ID.getName(),
				testCondition.getColumnName());
		TestCase.assertEquals("=", testCondition.getOperator());
		TestCase.assertEquals("'123'", testCondition.getValue());
	}

	@Test
	public final void testTranslateQueryCheckingIRODSQueryFieldsType()
			throws Exception {
		String query = ("select "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ","
				+ RodsGenQueryEnum.COL_AUDIT_USER_ID.getName() + " where "
				+ RodsGenQueryEnum.COL_AUDIT_OBJ_ID.getName() + " = '123'");

		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);

		TestCase.assertEquals(1, translatedQuery.getTranslatedQueryConditions()
				.size());
		TranslatedQueryCondition testCondition = translatedQuery
				.getTranslatedQueryConditions().get(0);
		TestCase.assertEquals("this should be an irods gen query field",
				SelectField.SelectFieldSource.DEFINED_QUERY_FIELD,
				testCondition.getFieldSource());
	}

	@Test(expected = JargonQueryException.class)
	public final void testTranslateQueryMissingWhere() throws Exception {
		StringBuilder query = new StringBuilder();

		query.append("SELECT ");
		query.append(RodsGenQueryEnum.COL_USER_GROUP_ID.getName());
		query.append(',');
		query.append(RodsGenQueryEnum.COL_USER_GROUP_NAME.getName());
		query.append(',');
		query.append(RodsGenQueryEnum.COL_USER_NAME.getName());
		query.append(" = '");
		query.append("joebob");
		query.append("'");
		String queryString = query.toString();
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(queryString, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
	}

	@Test(expected = JargonQueryException.class)
	public final void testTranslateQueryMissingComma() throws Exception {
		StringBuilder query = new StringBuilder();

		query.append("SELECT ");
		query.append(RodsGenQueryEnum.COL_USER_GROUP_ID.getName());
		query.append(',');
		query.append(RodsGenQueryEnum.COL_USER_GROUP_NAME.getName());
		query.append(RodsGenQueryEnum.COL_USER_NAME.getName());
		query.append(" = '");
		query.append("joebob");
		query.append("'");
		String queryString = query.toString();
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(queryString, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
	}

	@Test
	public final void testTranslateQueryCheckingIRODSQueryFieldTranslation()
			throws Exception {
		String query = ("select "
				+ RodsGenQueryEnum.COL_DATA_ACCESS_DATA_ID.getName() + ","
				+ RodsGenQueryEnum.COL_AUDIT_USER_ID.getName() + " where "
				+ RodsGenQueryEnum.COL_AUDIT_OBJ_ID.getName() + " = '123'");

		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);

		TestCase.assertEquals(1, translatedQuery.getTranslatedQueryConditions()
				.size());
		TranslatedQueryCondition testCondition = translatedQuery
				.getTranslatedQueryConditions().get(0);
		TestCase.assertEquals(
				"this should be looked up and translated to irods code", String
						.valueOf(RodsGenQueryEnum.COL_AUDIT_OBJ_ID
								.getNumericValue()), testCondition
						.getColumnNumericTranslation());
	}
	
	@Test
	public final void queryExample1Test() throws Exception {
		String query = "SELECT COLL_ID,COLL_NAME,META_COLL_ATTR_NAME,META_COLL_ATTR_VALUE,META_COLL_ATTR_UNITS WHERE META_COLL_ATTR_NAME = 'PolicyDrivenService:PolicyRepository' AND META_COLL_ATTR_VALUE = 'My first policy'";
		IRODSServerProperties props = IRODSServerProperties.instance(
				IRODSServerProperties.IcatEnabled.ICAT_ENABLED, 100, "rods2.2",
				"d", "zone");
		IRODSQueryTranslator translator = new IRODSQueryTranslator(props);

		IRODSQuery irodsQuery = IRODSQuery.instance(query, 10);

		TranslatedIRODSQuery translatedQuery = translator
				.getTranslatedQuery(irodsQuery);
	}
}
