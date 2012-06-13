/**
 * 
 */
package org.irods.jargon.core.connection;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.packinstr.MiscSvrInfo;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.IRODSCommands;
import edu.sdsc.grid.io.irods.Tag;

/**
 * Obtain information about the connected irods server.
 * @author Mike Conway - DICE (www.irods.org)
 *
 */
public class EnvironmentalInfoAccessor {
	
	private IRODSCommands irodsProtocol = null;
	private Logger log = LoggerFactory.getLogger(EnvironmentalInfoAccessor.class);

	
	public EnvironmentalInfoAccessor(final IRODSCommands irodsProtocol) throws JargonException {
		if (irodsProtocol == null) {
			throw new JargonException("null irodsProtocol");
		}
		if (!irodsProtocol.isConnected()) {
			throw new JargonException("irods protocol is not connected");
		}
		this.irodsProtocol = irodsProtocol;
		
	}

	public IRODSServerProperties getIRODSServerProperties() throws JargonException {
		log.info("getting irods server properties");
		Tag response = irodsProtocol.irodsFunction(MiscSvrInfo.PI_TAG,"",MiscSvrInfo.API_NBR);
		int serverType = response.getTag(MiscSvrInfo.SERVER_TYPE_TAG).getIntValue();
		
		IRODSServerProperties.IcatEnabled icatEnabled = null;
		if (serverType == 1) {
			icatEnabled = IRODSServerProperties.IcatEnabled.ICAT_ENABLED;
		} else {
			icatEnabled = IRODSServerProperties.IcatEnabled.NO_ICAT;
		}
		
		int serverBootTime = response.getTag(MiscSvrInfo.SERVER_BOOT_TIME_TAG).getIntValue();
		String relVersion = response.getTag(MiscSvrInfo.REL_VERSION_TAG).getStringValue();
		String apiVersion = response.getTag(MiscSvrInfo.API_VERSION_TAG).getStringValue();
		String rodsZone = response.getTag(MiscSvrInfo.RODS_ZONE_TAG).getStringValue();
		return IRODSServerProperties.instance(icatEnabled, serverBootTime, relVersion, apiVersion, rodsZone);
	}
	
}
