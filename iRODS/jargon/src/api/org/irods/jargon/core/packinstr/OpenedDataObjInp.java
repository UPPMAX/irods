/**
 * 
 */
package org.irods.jargon.core.packinstr;

import java.util.ArrayList;
import java.util.List;

import org.irods.jargon.core.exception.JargonException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.Tag;

/**
 * Translation of an OpenedDataObjInp operation into XML protocol format.  This is a 'newer' version of a seek operation.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */


public class OpenedDataObjInp extends AbstractIRODSPackingInstruction {

	public static final String PI_TAG = "OpenedDataObjInp_PI";
	public static final String L1_DESC_INX = "l1descInx";
	public static final String LEN = "len";
	public static final String WHENCE = "whence";
	public static final String OPR_TYPE = "oprType";
	public static final String OFFSET = "offset";
	public static final String BYTES_WRITTEN = "bytesWritten";

	public static final int SEEK_API_NBR = 674;
	public static final int DEFAULT_OPERATION_TYPE = 0;
	private static final Logger log = LoggerFactory.getLogger(OpenedDataObjInp.class);

	private final long offset;
	private final int operationType = DEFAULT_OPERATION_TYPE;
	private final int fileDescriptor;
	private final int whence;
	
	public static final int SEEK_START = 0;
	public static final int SEEK_CURRENT = 1;
	public static final int SEEK_END = 2;

	/**
	 * Create an instance of the OpenedDataObjInp packing instruction for a file seek operation
	 * @param offset <code>long</code> with the offset into the file to seek to.
	 * @param fileDescriptor <code>int</code> that iRODS assigns to the file when opening.
	 * @param whence <code>int</code> with the proper seek type (see SEEK_ constants in this class)
	 * @return an instance of the packing instruction ready to send to iRODS.
	 * @throws JargonException
	 */
	public static final OpenedDataObjInp instanceForFileSeek(final long offset, final int fileDescriptor, final int whence) throws JargonException {
		return new OpenedDataObjInp(offset, fileDescriptor, whence);
	}
	
	
	private OpenedDataObjInp(final long offset, final int fileDescriptor, final int whence) throws JargonException {
		if (offset < 0) {
			throw new JargonException("offset is less than zero");
		}
		
		if (fileDescriptor <= 0) {
			throw new JargonException("fileDescriptor must be > 0");
		}
		
		if (whence < 0 || whence > 2) {
			throw new JargonException("invalid whence value");
		}
		
		this.offset = offset;
		this.fileDescriptor = fileDescriptor;
		this.setApiNumber(SEEK_API_NBR);
		this.whence = whence;
	}
	public Tag getTagValue() throws JargonException {
		

		Tag message = new Tag(PI_TAG, new Tag[] {
				new Tag(L1_DESC_INX, getFileDescriptor()),
				new Tag(LEN, 0),
				new Tag(WHENCE, whence),
				new Tag(OPR_TYPE, getOperationType()),
				new Tag(OFFSET, getOffset()),
				new Tag(BYTES_WRITTEN, 0)
			 });

		List<KeyValuePair> kvps = new ArrayList<KeyValuePair>();
		message.addTag(createKeyValueTag(kvps));
		return message;
	}

	public long getOffset() {
		return offset;
	}

	public int getOperationType() {
		return operationType;
	}

	public int getFileDescriptor() {
		return fileDescriptor;
	}
	public int getWhence() {
		return whence;
	}

	

}
