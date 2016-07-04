/*******************************************************************************

Licensed to the OpenCOR team under one or more contributor license agreements.
See the NOTICE.txt file distributed with this work for additional information
regarding copyright ownership. The OpenCOR team licenses this file to you under
the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied. See the License for the
specific language governing permissions and limitations under the License.

*******************************************************************************/

//==============================================================================
// PMR workspace
//==============================================================================

#include "corecliutils.h"
#include "coreguiutils.h"
#include "pmrworkspace.h"
#include "pmrworkspacesmanager.h"

//==============================================================================

#include <Qt>

//==============================================================================

#include <QDir>
#include <QStandardPaths>

//==============================================================================

namespace OpenCOR {
namespace PMRSupport {

//==============================================================================

PmrWorkspace::PmrWorkspace(QObject *parent) : QObject(parent), mOwned(false),
    mDescription(QString()), mName(QString()), mOwner(QString()), mUrl(QString()),
    mPassword(QString()), mUsername(QString()), mGitRepository(nullptr), mPath(QString())
{
}

//==============================================================================

PmrWorkspace::PmrWorkspace(const QString &pUrl, const QString &pName, QObject *parent) :
    QObject(parent), mOwned(false),
    mDescription(QString()), mName(pName), mOwner(QString()), mUrl(pUrl),
    mPassword(QString()), mUsername(QString()), mGitRepository(nullptr), mPath(QString())
{
    // Name, description and owner are set from PMR workspace info
}

//==============================================================================

PmrWorkspace::~PmrWorkspace()
{
    close();
}

//==============================================================================

bool PmrWorkspace::isLocal(void) const
{
    return !mPath.isNull();
  }

//==============================================================================

bool PmrWorkspace::isNull(void) const
{
    return mUrl.isNull();
  }

//==============================================================================

bool PmrWorkspace::compare(const PmrWorkspace *pFirst, const PmrWorkspace *pSecond)
{
    // Return whether the first workspace is lower than the second one (without
    // worrying about casing)

    return pFirst->name().compare(pSecond->name(), Qt::CaseInsensitive) < 0;
}

//==============================================================================

QString PmrWorkspace::WorkspacesDirectory()
{
    static QString workspacesHome = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                                  + QDir::separator() + "OpenCOR" + QDir::separator() + "Workspaces";

    static bool checkedFolder = false;
    if (!checkedFolder) {
        auto workspacesFolder = QDir(workspacesHome);
        if (!workspacesFolder.exists()) workspacesFolder.mkpath(".");
        checkedFolder = true;
    }

    return workspacesHome;
}

//==============================================================================

QString PmrWorkspace::getEmptyWorkspaceDirectory(void)
{
    // Retrieve the name of an empty directory

    return Core::getExistingDirectory(tr("Select Empty Directory"),
                                      PMRSupport::PmrWorkspace::WorkspacesDirectory(),
                                      true);
}

//==============================================================================

bool PmrWorkspace::isOwned(void) const
{
    return mOwned;
}

//==============================================================================

void PmrWorkspace::setOwned(const bool &pOwned)
{
    mOwned = pOwned;
}

//==============================================================================

void PmrWorkspace::setCredentials(const QString &pUsername, const QString &pPassword)
{
    mUsername = pUsername;
    mPassword = pPassword;
}

//==============================================================================

void PmrWorkspace::setDescription(const QString &pDescription)
{
    mDescription = pDescription;
}

//==============================================================================

void PmrWorkspace::setName(const QString &pName)
{
    mName = pName;
}

//==============================================================================

void PmrWorkspace::setOwner(const QString &pOwner)
{
    mOwner = pOwner;
}

//==============================================================================

void PmrWorkspace::setPath(const QString &pPath)
{
    mPath = pPath;
}

//==============================================================================

const QString &PmrWorkspace::description() const
{
    return mDescription;
}

//==============================================================================

const QString &PmrWorkspace::name() const
{
    // Return our name

    return mName;
}

//==============================================================================

const QString &PmrWorkspace::owner() const
{
    return mOwner;
}

//==============================================================================

const QString &PmrWorkspace::path() const
{
    return mPath;
}

//==============================================================================

const QString &PmrWorkspace::url() const
{
    // Return our URL

    return mUrl;
}

//==============================================================================

void PmrWorkspace::emitProgress(const double &pProgress) const
{
    emit progress(pProgress);
}

//==============================================================================

void PmrWorkspace::emitGitError(const QString &pMessage) const
{
    const git_error *gitError = giterr_last();

    emit warning(pMessage + (gitError ? (QString("<br></br>")
                                         + tr("Error %1: %2.").arg(QString::number(gitError->klass),
                                                                   Core::formatMessage(gitError->message)))
                                      : ""));
}

//==============================================================================

bool PmrWorkspace::open(void)
{
    close();
    if (!mPath.isEmpty()) {
        if (mGitRepository == nullptr) {
            if (git_repository_open(&mGitRepository, mPath.toUtf8().constData())) {
                emitGitError(tr("An error occurred while trying to open the workspace."));
                return false;
            }
            return true;
        }
    }
    return false;
}

//==============================================================================

bool PmrWorkspace::opened(void) const
{
    return (mGitRepository != nullptr);
}

//==============================================================================

void PmrWorkspace::close(void)
{
    if (mGitRepository != nullptr) {
        git_repository_free(mGitRepository);
        mGitRepository = nullptr;
    }
}

//==============================================================================

void PmrWorkspace::setGitAuthorisation(git_strarray *pAuthorisationStrArray)
{
    if (!mUsername.isEmpty()) {
        // Initialise a git_strarray with a Basic Authorization header

        auto authorisationHeader = QByteArray();
        authorisationHeader.append("Authorization: Basic ");
        authorisationHeader.append((mUsername + ":" + mPassword).toUtf8().toBase64());

        char *authorisationStrArrayData = (char *)malloc(authorisationHeader.count() + 1);
        memcpy(authorisationStrArrayData, authorisationHeader.constData(), authorisationHeader.count() + 1);

        char **authorisationStrArrayArray = (char **)malloc(sizeof(char *));
        authorisationStrArrayArray[0] = authorisationStrArrayData;

        pAuthorisationStrArray->strings = authorisationStrArrayArray;
        pAuthorisationStrArray->count = 1;
    }
}

//==============================================================================

QString PmrWorkspace::getUrlFromFolder(const QString &pFolder)
{
    // Return remote.origin.url if the folder contains a git repository
    // otherwise return an empty string.

    auto url = QString();

    git_repository *gitRepository = 0;
    if (git_repository_open(&gitRepository, pFolder.toUtf8().constData()) == 0) {

        git_strarray remotes ;
        if (git_remote_list(&remotes, gitRepository) == 0) {

            for (int i = 0; i < (int)remotes.count; i++) {
                char *name = remotes.strings[i];
                if (strcmp(name, "origin")==0) {
                    git_remote *remote = {0};

                    if (git_remote_lookup(&remote, gitRepository, name) == 0) {
                        const char *remoteUrl = git_remote_url(remote);
                        if (remoteUrl) url = QString(remoteUrl);
                    }
                }
            }
        }
        git_repository_free(gitRepository);
    }
    return url;
}

//==============================================================================

// See https://libgit2.github.com/libgit2/#HEAD/type/git_remote_callbacks

// If cert verification fails, this is called to let the user make the final
// decision of whether to allow the connection to proceed. Returns 1 to allow
// the connection, 0 to disallow it or a negative value to indicate an error.

int PmrWorkspace::certificate_check_cb(git_cert *cert, int valid, const char *host, void *payload)
{
    Q_UNUSED(cert)
    Q_UNUSED(valid)
    Q_UNUSED(host)
    Q_UNUSED(payload)

    // pmrRepository.Url().compare(host)
    return 1; // since we trust PMR (but should check host matches PMR...)
              // 0 = disallow, -ve = error
}

//==============================================================================

int PmrWorkspace::transfer_progress_cb(const git_transfer_progress *stats, void *payload)
{
    auto workspace = (PmrWorkspace *)payload;

    workspace->emitProgress((double)(stats->received_objects + stats->indexed_objects)
                            /(2.0*stats->total_objects));
    return 0;
}

//==============================================================================

void PmrWorkspace::checkout_progress_cb(const char *path, size_t completed_steps, size_t total_steps,
                                        void *payload)
{
    Q_UNUSED(path)

    auto workspace = (PmrWorkspace *)payload;

    workspace->emitProgress((double)completed_steps/(double)total_steps);
}

//==============================================================================

void PmrWorkspace::clone(const QString &pDirName)
{
    // Clone a workspace

    QByteArray workspaceByteArray = mUrl.toUtf8();
    QByteArray dirNameByteArray = pDirName.toUtf8();

    git_clone_options cloneOptions;
    git_clone_init_options(&cloneOptions, GIT_CLONE_OPTIONS_VERSION);

    // We trust PMR's SSL certificate

    cloneOptions.fetch_opts.callbacks.certificate_check = certificate_check_cb;

    // Track clone and checkout progress

    cloneOptions.fetch_opts.callbacks.transfer_progress = transfer_progress_cb ;
    cloneOptions.fetch_opts.callbacks.payload = (void *)this;
    cloneOptions.checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    cloneOptions.checkout_opts.progress_cb = checkout_progress_cb;
    cloneOptions.checkout_opts.progress_payload = (void *)this;

    // Set up Basic authorization

    git_strarray authorisationStrArray = { nullptr, 0 };
    setGitAuthorisation(&authorisationStrArray);
    cloneOptions.fetch_opts.custom_headers = authorisationStrArray;

    // Perform the clone

    if (git_clone(&mGitRepository, workspaceByteArray.constData(),
                          dirNameByteArray.constData(), &cloneOptions))
        emitGitError(tr("An error occurred while trying to clone the workspace."));

    git_strarray_free(&authorisationStrArray);

    mPath = pDirName;

    PmrWorkspacesManager::instance()->emitWorkspaceCloned(this);
    emit workspaceCloned(this);
}

//==============================================================================

void PmrWorkspace::push(void)
{
    // Push a workspace

    git_push_options pushOptions;
    git_push_init_options(&pushOptions, GIT_PUSH_OPTIONS_VERSION);

    // We trust PMR's SSL certificate

    pushOptions.callbacks.certificate_check = certificate_check_cb;

    // Track push progress

    pushOptions.callbacks.transfer_progress = transfer_progress_cb ;
    pushOptions.callbacks.payload = (void *)this;

    // Set up Basic authorization

    git_strarray authorisationStrArray = { nullptr, 0 };
    setGitAuthorisation(&authorisationStrArray);
    pushOptions.custom_headers = authorisationStrArray;

    if (this->open()) {

        git_remote_callbacks remoteCallbacks;
        git_remote_init_callbacks(&remoteCallbacks, GIT_REMOTE_CALLBACKS_VERSION);
        remoteCallbacks.certificate_check = certificate_check_cb;

        // Get the remote, connect to it, add a refspec, and do the push

        git_remote *gitRemote = nullptr;
        if (git_remote_add_push(mGitRepository, "origin", "refs/heads/master:refs/heads/master")
         || git_remote_lookup(&gitRemote, mGitRepository, "origin")
         || git_remote_connect(gitRemote, GIT_DIRECTION_PUSH, &remoteCallbacks, &authorisationStrArray)
         || git_remote_push(gitRemote, nullptr, &pushOptions)) {
            emitGitError(tr("An error occurred while trying to push the workspace."));
        if (gitRemote) git_remote_free(gitRemote);
        }
    }

    git_strarray_free(&authorisationStrArray);

    emit workspacePushed(this);
}

/*
if (this->open()) {

    git_index *index;
    // Get the index to the repository
    git_repository_index(&index, mGitRepository);

    // Add modified files
    git_index_add_bypath(&index, "path/relative/to/working/dir/of/file");
    // Remove deleted files
    git_index_remove_bypath(&index, "path/relative/to/working/dir/of/file");

    git_tree treeId;
    git_index_write_tree(&treeId, index);

    git_index_free(index);

    git_tree *tree;
    git_tree_lookup(&tree, mGitRepository, &treeId);

    git_oid parentId;

    // Get HEAD as a commit object to use as the parent of the commit
    git_reference_name_to_id(parentId, mGitRepository, "HEAD");

    git_commit *parent;
    git_commit_lookup(&parent, mGitRepository, parentId);

    gir_oid commitId;

    // Do the commit
    git_commit_create_v(
        &commitId,
        mGitRepository,
        "HEAD",           // The commit will update the position of HEAD
        author,
        committer,
        NULL,             // UTF-8 encoding
        message,
        tree,             // The tree from the index
        1,                // Only one parent
        parent            // No need to make a list with create_v
    );

    git_tree_free(tree);
    git_commit_free(parent);
}

*/

//==============================================================================

PmrWorkspace::RemoteStatus PmrWorkspace::gitRemoteStatus(void) const
{
    // Get the status of the repository

    auto status = StatusUnknown;

    if (this->opened()) {
        git_oid masterOid, originMasterOid;

        if (git_reference_name_to_id(&masterOid, mGitRepository,
                                                 "refs/heads/master") == 0
         && git_reference_name_to_id(&originMasterOid, mGitRepository,
                                                 "refs/remotes/origin/master") == 0) {
            size_t ahead = 0;
            size_t behind = 0;

            if (git_graph_ahead_behind(&ahead, &behind, mGitRepository,
                                       &masterOid, &originMasterOid) == 0) {
                status = behind ? StatusBehind
                       : ahead  ? StatusAhead
                       :          StatusCurrent;
// TODO If there are uncommitted files or files in the index (to be committed)
//      then return StatusCommit
            }
        }
        if (status == StatusUnknown)
            emitGitError(tr("An error occurred while trying to get the status of %1").arg(mPath));
    }

    return status;
}

//==============================================================================

const QList<QChar> PmrWorkspace::gitFileStatus(const QString &pPath) const
{
    // Get the status of a file

    auto status = QList<QChar>() << ' ' << ' ';
//qDebug() << "Get status: " << pPath;
    if (this->opened()) {
        auto repoDir = QDir(mPath);
        auto relativePath = repoDir.relativeFilePath(pPath);

        unsigned int statusFlags = 0;
        if (git_status_file(&statusFlags, mGitRepository, relativePath.toUtf8().constData()) == 0) {
            char istatus = ' ';
            char wstatus = ' ';
//qDebug() << "   Flags: " << statusFlags;

            if (statusFlags & GIT_STATUS_INDEX_NEW) istatus = 'A';
            if (statusFlags & GIT_STATUS_INDEX_MODIFIED) istatus = 'M';
            if (statusFlags & GIT_STATUS_INDEX_DELETED) istatus = 'D';
            if (statusFlags & GIT_STATUS_INDEX_RENAMED) istatus = 'R';
            if (statusFlags & GIT_STATUS_INDEX_TYPECHANGE) istatus = 'T';
            if (statusFlags & GIT_STATUS_WT_NEW) {
                if (istatus == ' ') istatus = '?';
                wstatus = '?';
            }
            if (statusFlags & GIT_STATUS_WT_MODIFIED) wstatus = 'M';
            if (statusFlags & GIT_STATUS_WT_DELETED) wstatus = 'D';
            if (statusFlags & GIT_STATUS_WT_RENAMED) wstatus = 'R';
            if (statusFlags & GIT_STATUS_WT_TYPECHANGE) wstatus = 'T';
            if (statusFlags & GIT_STATUS_IGNORED) {
                istatus = '!';
                wstatus = '!';
            }
            status[0] = istatus;
            status[1] = wstatus;
// TODO Combine istatus and wstatus?? A/M/D/R/T/?/!
//      What has precedence? Index? "AM" --> "A" but "AD" --> "D"
//      Whatever the result after adding the work file?? Then "MM" could goto "".
        }
        else {
            emitGitError(tr("An error occurred while trying to get the status of %1").arg(pPath));
        }
    }

    return status;
}

//==============================================================================

void PmrWorkspace::stageFile(const QString &pPath, const bool &pStage)
{
    if (this->open()) {
        auto repoDir = QDir(mPath);
        auto relativePath = repoDir.relativeFilePath(pPath);

        git_index *index;
        git_repository_index(&index, mGitRepository);

        if (pStage) git_index_add_bypath(index, relativePath.toUtf8().constData());
        else        git_index_remove_bypath(index, relativePath.toUtf8().constData());

        git_index_write(index);

// TODO check return values
        git_index_free(index);
    }
}

//==============================================================================
//==============================================================================

PmrWorkspaceList::PmrWorkspaceList() :
    QList<PmrWorkspace *>()
{
}

//==============================================================================

void PmrWorkspaceList::add(const QString &pUrl, const QString &pName, QObject *parent)
{
    // Add a new workspace to the list

    QList::append(new PmrWorkspace(pUrl, pName, parent));
}

//==============================================================================

}   // namespace PMRSupport
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
