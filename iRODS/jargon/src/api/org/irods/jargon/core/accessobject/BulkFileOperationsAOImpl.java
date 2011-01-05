package org.irods.jargon.core.accessobject;

import org.irods.jargon.core.exception.JargonException;
import org.irods.jargon.core.packinstr.StructFileExtAndRegInp;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.sdsc.grid.io.irods.IRODSCommands;

/**
 * Object to handle bundled file operations. This object contains functionality
 * similar to the iRODS <code>ibun</command> to transmit and register, or 
 * bundle and receive files.
 * 
 * @author Mike Conway - DICE (www.irods.org)
 * 
 */
public class BulkFileOperationsAOImpl  extends AbstractIRODSAccessObject implements
		BulkFileOperationsAO {

	public static final Logger log = LoggerFactory
			.getLogger(BulkFileOperationsAOImpl.class);

	/**
	 * Constructor as called by the <code>IRODSAccessObjectFactory</code>, which
	 * is properly used to construct this access object.
	 * 
	 * @param irodsSession
	 * @param irodsAccount
	 * @throws JargonException
	 */
	protected BulkFileOperationsAOImpl(final IRODSCommands irodsCommands)
			throws JargonException {
		super(irodsCommands);
	}


	//@Override
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.BulkFileOperationsAO#createABundleFromIrodsFilesAndStoreInIrods(java.lang.String, java.lang.String, java.lang.String)
	 */
	public void createABundleFromIrodsFilesAndStoreInIrods(
			final String absolutePathToBundleFileToBeCreatedOnIrods,
			final String absolutePathToIrodsCollectionToBeBundled,
			final String resourceNameWhereBundleWillBeStored)
			throws JargonException {

		if (absolutePathToBundleFileToBeCreatedOnIrods == null
				|| absolutePathToBundleFileToBeCreatedOnIrods.isEmpty()) {
			throw new IllegalArgumentException(
					"null or empty absolutePathToBundleFileToBeCreatedOnIrods");
		}

		if (absolutePathToIrodsCollectionToBeBundled == null
				|| absolutePathToIrodsCollectionToBeBundled.isEmpty()) {
			throw new IllegalArgumentException(
					"null or empty absolutePathToIrodsCollectionToBeBundled");
		}

		if (resourceNameWhereBundleWillBeStored == null) {
			throw new IllegalArgumentException(
					"null resourceNameWhereBundleWillBeStored. set to blank if not used");
		}

		log.info("createABundleFromIrodsFilesAndStoreInIrods, tar file:{}",
				absolutePathToBundleFileToBeCreatedOnIrods);
		log.info("source collection for tar:{}",
				absolutePathToIrodsCollectionToBeBundled);
		log.info("resource:{}", resourceNameWhereBundleWillBeStored);
		StructFileExtAndRegInp structFileExtAndRegInp = StructFileExtAndRegInp
				.instanceForCreateBundle(
						absolutePathToBundleFileToBeCreatedOnIrods,
						absolutePathToIrodsCollectionToBeBundled,
						resourceNameWhereBundleWillBeStored);

		this.getIrodsCommands().irodsFunction(structFileExtAndRegInp);

	}

	
	//@Override
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.BulkFileOperationsAO#createABundleFromIrodsFilesAndStoreInIrodsWithForceOption(java.lang.String, java.lang.String, java.lang.String)
	 */
	public void createABundleFromIrodsFilesAndStoreInIrodsWithForceOption(
			final String absolutePathToBundleFileToBeCreatedOnIrods,
			final String absolutePathToIrodsCollectionToBeBundled,
			final String resourceNameWhereBundleWillBeStored)
			throws JargonException {

		if (absolutePathToBundleFileToBeCreatedOnIrods == null
				|| absolutePathToBundleFileToBeCreatedOnIrods.isEmpty()) {
			throw new IllegalArgumentException(
					"null or empty absolutePathToBundleFileToBeCreatedOnIrods");
		}

		if (absolutePathToIrodsCollectionToBeBundled == null
				|| absolutePathToIrodsCollectionToBeBundled.isEmpty()) {
			throw new IllegalArgumentException(
					"null or empty absolutePathToIrodsCollectionToBeBundled");
		}

		if (resourceNameWhereBundleWillBeStored == null) {
			throw new IllegalArgumentException(
					"null resourceNameWhereBundleWillBeStored. set to blank if not used");
		}

		log.info("createABundleFromIrodsFilesAndStoreInIrods, tar file:{}",
				absolutePathToBundleFileToBeCreatedOnIrods);
		log.info("source collection for tar:{}",
				absolutePathToIrodsCollectionToBeBundled);
		log.info("resource:{}", resourceNameWhereBundleWillBeStored);
		StructFileExtAndRegInp structFileExtAndRegInp = StructFileExtAndRegInp
				.instanceForCreateBundleWithForceOption(
						absolutePathToBundleFileToBeCreatedOnIrods,
						absolutePathToIrodsCollectionToBeBundled,
						resourceNameWhereBundleWillBeStored);

		this.getIrodsCommands().irodsFunction(structFileExtAndRegInp);

	}

	
	//@Override
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.BulkFileOperationsAO#extractABundleIntoAnIrodsCollection(java.lang.String, java.lang.String, java.lang.String)
	 */
	public void extractABundleIntoAnIrodsCollection(
			final String absolutePathToBundleFileInIrodsToBeExtracted,
			final String absolutePathToIrodsCollectionToHoldExtractedFiles,
			final String resourceNameWhereBundleWillBeExtracted)
			throws JargonException {

		extractABundleIntoAnIrodsCollection(
				absolutePathToBundleFileInIrodsToBeExtracted,
				absolutePathToIrodsCollectionToHoldExtractedFiles,
				resourceNameWhereBundleWillBeExtracted, false, false);
	}


	//@Override
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.BulkFileOperationsAO#extractABundleIntoAnIrodsCollectionWithBulkOperationOptimization(java.lang.String, java.lang.String, java.lang.String)
	 */
	public void extractABundleIntoAnIrodsCollectionWithBulkOperationOptimization(
			final String absolutePathToBundleFileInIrodsToBeExtracted,
			final String absolutePathToIrodsCollectionToHoldExtractedFiles,
			final String resourceNameWhereBundleWillBeExtracted)
			throws JargonException {

		extractABundleIntoAnIrodsCollection(
				absolutePathToBundleFileInIrodsToBeExtracted,
				absolutePathToIrodsCollectionToHoldExtractedFiles,
				resourceNameWhereBundleWillBeExtracted, false, true);
	}

	
	//@Override
	/* (non-Javadoc)
	 * @see org.irods.jargon.core.accessobject.BulkFileOperationsAO#extractABundleIntoAnIrodsCollectionWithForceOption(java.lang.String, java.lang.String, java.lang.String)
	 */
	public void extractABundleIntoAnIrodsCollectionWithForceOption(
			final String absolutePathToBundleFileInIrodsToBeExtracted,
			final String absolutePathToIrodsCollectionToHoldExtractedFiles,
			final String resourceNameWhereBundleWillBeExtracted)
			throws JargonException {

		extractABundleIntoAnIrodsCollection(
				absolutePathToBundleFileInIrodsToBeExtracted,
				absolutePathToIrodsCollectionToHoldExtractedFiles,
				resourceNameWhereBundleWillBeExtracted, true, false);
	}

	/**
	 * Internal method with params for various options to be delegated to by
	 * specific extract methods in api
	 * 
	 * @param absolutePathToBundleFileInIrodsToBeExtracted
	 * @param absolutePathToIrodsCollectionToHoldExtractedFiles
	 * @param resourceNameWhereBundleWillBeExtracted
	 * @param force
	 * @param bulkOperation
	 * @throws JargonException
	 */
	protected void extractABundleIntoAnIrodsCollection(
			final String absolutePathToBundleFileInIrodsToBeExtracted,
			final String absolutePathToIrodsCollectionToHoldExtractedFiles,
			final String resourceNameWhereBundleWillBeExtracted,
			final boolean force, final boolean bulkOperation)
			throws JargonException {

		if (absolutePathToBundleFileInIrodsToBeExtracted == null
				|| absolutePathToIrodsCollectionToHoldExtractedFiles.isEmpty()) {
			throw new IllegalArgumentException(
					"null or empty absolutePathToIrodsCollectionToHoldExtractedFiles");
		}

		if (absolutePathToIrodsCollectionToHoldExtractedFiles == null
				|| absolutePathToIrodsCollectionToHoldExtractedFiles.isEmpty()) {
			throw new IllegalArgumentException(
					"null or empty absolutePathToIrodsCollectionToHoldExtractedFiles");
		}

		if (resourceNameWhereBundleWillBeExtracted == null) {
			throw new IllegalArgumentException(
					"null or empty resourceNameWhereBundleWillBeExtracted, set to blank if not used");
		}

		log.info("extractABundleIntoAnIrodsCollection, tar file:{}",
				absolutePathToBundleFileInIrodsToBeExtracted);
		log.info("target collection for tar expansio:{}",
				absolutePathToIrodsCollectionToHoldExtractedFiles);
		log.info("resource:{}", resourceNameWhereBundleWillBeExtracted);

		StructFileExtAndRegInp structFileExtAndRegInp;

		if (force) {
			if (bulkOperation) {
				log.info("force, bulk optimization");
				structFileExtAndRegInp = StructFileExtAndRegInp
						.instanceForExtractBundleWithForceOptionAndBulkOperation(
								absolutePathToBundleFileInIrodsToBeExtracted,
								absolutePathToIrodsCollectionToHoldExtractedFiles,
								resourceNameWhereBundleWillBeExtracted);
			} else {
				log.info("force, no bulk optimization");
				structFileExtAndRegInp = StructFileExtAndRegInp
						.instanceForExtractBundleWithForceOption(
								absolutePathToBundleFileInIrodsToBeExtracted,
								absolutePathToIrodsCollectionToHoldExtractedFiles,
								resourceNameWhereBundleWillBeExtracted);
			}
		} else {
			if (bulkOperation) {
				log.info("no force, bulk optimization");
				structFileExtAndRegInp = StructFileExtAndRegInp
						.instanceForExtractBundleNoForceWithBulkOperation(
								absolutePathToBundleFileInIrodsToBeExtracted,
								absolutePathToIrodsCollectionToHoldExtractedFiles,
								resourceNameWhereBundleWillBeExtracted);
			} else {
				log.info("no force, no bulk optimization");
				structFileExtAndRegInp = StructFileExtAndRegInp
						.instanceForExtractBundleNoForce(
								absolutePathToBundleFileInIrodsToBeExtracted,
								absolutePathToIrodsCollectionToHoldExtractedFiles,
								resourceNameWhereBundleWillBeExtracted);
			}
		}

		this.getIrodsCommands().irodsFunction(structFileExtAndRegInp);

	}

}
