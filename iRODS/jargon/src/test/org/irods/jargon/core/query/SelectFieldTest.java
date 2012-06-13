package org.irods.jargon.core.query;

import junit.framework.Assert;
import org.irods.jargon.core.exception.JargonException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

public class SelectFieldTest {

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
	}

	@Test
	public final void testInstanceRodsGenQueryEnumSelectFieldTypesSelectFieldSource()
			throws Exception {
		SelectField selectField = SelectField.instance(
				RodsGenQueryEnum.COL_AUDIT_ACTION_ID,
				SelectField.SelectFieldTypes.FIELD,
				SelectField.SelectFieldSource.DEFINED_QUERY_FIELD);
		Assert.assertNotNull("null instance", selectField);
	}

	@Test(expected = JargonException.class)
	public final void testInstanceNullRodsGenQueryEnum() throws Exception {
		SelectField.instance(null, SelectField.SelectFieldTypes.FIELD,
				SelectField.SelectFieldSource.DEFINED_QUERY_FIELD);
	}

	@Test(expected = JargonException.class)
	public final void testInstanceNullSelectFieldType() throws Exception {
		SelectField.instance(RodsGenQueryEnum.COL_AUDIT_ACTION_ID, null,
				SelectField.SelectFieldSource.DEFINED_QUERY_FIELD);
	}

	@Test(expected = JargonException.class)
	public final void testInstanceNullSelectFieldSource() throws Exception {
		SelectField.instance(RodsGenQueryEnum.COL_AUDIT_ACTION_ID,
				SelectField.SelectFieldTypes.FIELD, null);
	}

	@Test
	public final void testInstanceWithNameAndType() throws Exception {
		SelectField selectField = SelectField.instance("blah", "1234",
				SelectField.SelectFieldTypes.FIELD,
				SelectField.SelectFieldSource.DEFINED_QUERY_FIELD);
		Assert.assertNotNull("null instance", selectField);
	}

}
