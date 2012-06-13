/**
 * 
 */
package org.irods.jargon.core.utils;

import java.util.ArrayList;
import java.util.List;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.pub.domain.AvuData;
import org.irods.jargon.core.query.IRODSQueryResultRow;
import org.irods.jargon.core.query.IRODSQueryResultSet;
import org.irods.jargon.core.query.MetaDataAndDomainData;
import org.irods.jargon.core.query.MetaDataAndDomainData.MetadataDomain;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Utility methods for processing various query operations for Jargon Access Objects.  These methods are for internal use and 
 * have limited utility outside of that.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class AccessObjectQueryProcessingUtils {
	private static Logger log = LoggerFactory.getLogger(AccessObjectQueryProcessingUtils.class);

	/**
	 * @param resultSet
	 * @return
	 * @throws JargonException
	 */
	public static List<AvuData> buildAvuDataListFromResultSet(
			IRODSQueryResultSet resultSet) throws JargonException {
		final List<AvuData> avuDatas = new ArrayList<AvuData>();
		AvuData avuData = null;

		if (resultSet.getNumberOfResultColumns() != 3) {
			final String msg = "number of results for avu query should be 3, was:"
					+ resultSet.getNumberOfResultColumns();
			log.error(msg);
			throw new JargonException(msg);
		}

		for (IRODSQueryResultRow row : resultSet.getResults()) {
			avuData = AvuData.instance(row.getColumn(0), row.getColumn(1), row
					.getColumn(2));
			avuDatas.add(avuData);
			if (log.isDebugEnabled()) {
				log.debug("found avu for user:" + avuData);
			}
		}
		return avuDatas;
	}
	
	/**
	 * @param metaDataDomain
	 * @param irodsQueryResultSet
	 * @return
	 * @throws JargonException
	 */
	public static  List<MetaDataAndDomainData> buildMetaDataAndDomainDatalistFromResultSet(
			final MetadataDomain metaDataDomain,
			final IRODSQueryResultSet irodsQueryResultSet)
			throws JargonException {
		if (metaDataDomain == null) {
			throw new JargonException("null metaDataDomain");
		}

		if (irodsQueryResultSet == null) {
			throw new JargonException("null irodsQueryResultSet");
		}

		List<MetaDataAndDomainData> metaDataResults = new ArrayList<MetaDataAndDomainData>();

		for (IRODSQueryResultRow row : irodsQueryResultSet.getResults()) {
			metaDataResults.add(buildMetaDataAndDomainDataFromResultSetRow(
					metaDataDomain, row));
		}

		return metaDataResults;
	}
	
	
	
	/**
	 * @param metadataDomain
	 * @param row
	 * @return
	 * @throws JargonException
	 */
	private static MetaDataAndDomainData buildMetaDataAndDomainDataFromResultSetRow(
			MetaDataAndDomainData.MetadataDomain metadataDomain,
			IRODSQueryResultRow row) throws JargonException {

		String domainId = row.getColumn(0);
		String domainUniqueName = row.getColumn(1);
		String attributeName = row.getColumn(2);
		String attributeValue = row.getColumn(3);
		String attributeUnits = row.getColumn(4);

		MetaDataAndDomainData data = MetaDataAndDomainData.instance(
				metadataDomain, domainId, domainUniqueName, attributeName,
				attributeValue, attributeUnits);
		log.debug("metadataAndDomainData: {}", data);
		return data;
	}




}
