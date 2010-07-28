/**
 * 
 */
package org.irods.jargon.core.query;

import java.util.List;

/**
 * Describes an element of an AVU query (e.g. attribute = some value, units like some value).  These are then used in groups 
 * to define a particular metadata query.
 * 
 * This is a partial implementation of the code, and currently is limited in usage.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class AVUQueryElement {
	public enum AVUQueryPart {ATTRIBUTE, UNITS, VALUE}
	public static AVUQueryElement instanceForValueQuery(final AVUQueryPart avuQueryPart, final AVUQueryOperatorEnum operator, final String value) throws JargonQueryException {
		return new AVUQueryElement(avuQueryPart, operator, value, null, null);
	}
	private final AVUQueryPart avuQueryPart;
	private final AVUQueryOperatorEnum operator;
	private final String value;
	private final String valueEndOfRange;
	
	private final List<Object> valuesTable;
	
	private AVUQueryElement(final AVUQueryPart avuQueryPart, final AVUQueryOperatorEnum operator, final String value, final String valueEndOfRange, final List<Object> valuesTable ) throws JargonQueryException {
		if (avuQueryPart == null) {
			throw new JargonQueryException("avuQueryPart is null");
		}
		
		if (operator == null) {
			throw new JargonQueryException("avuQueryOperator is null");
		}
		
		if (value == null || value.isEmpty()) {
			throw new JargonQueryException("null or empty value");
		}
		
		if (valueEndOfRange != null) {
			throw new JargonQueryException("currently unsupported");
		}
		
		if (valuesTable != null) {
			throw new JargonQueryException("currently unsupported");
		}
		
		this.avuQueryPart = avuQueryPart;
		this.operator = operator;
		this.value = value.trim();
		this.valueEndOfRange = valueEndOfRange;
		//this.valueEndOfRange = valueEndOfRange.trim();
		this.valuesTable = valuesTable;
	}
	
	public boolean equals(Object other) {
		if (this == other) {
			return true;
		}
		
		if (!(other instanceof AVUQueryElement)) {
			return false;
		}
		
		AVUQueryElement otherObj = (AVUQueryElement) other;

		return (avuQueryPart.equals(otherObj.avuQueryPart) &&
				operator.equals(otherObj.operator) &&
				value.equals(otherObj.value));
		
		// TODO expand as other options implemented
		
	}
	public AVUQueryPart getAvuQueryPart() {
		return avuQueryPart;
	}
	public AVUQueryOperatorEnum getOperator() {
		return operator;
	}
	public String getValue() {
		return value;
	}
	public String getValueEndOfRange() {
		return valueEndOfRange;
	}
	
	public List<Object> getValuesTable() {
		return valuesTable;
	}

	
}
