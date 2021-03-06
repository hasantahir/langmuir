#ifndef NODEFILEPARSER_H
#define NODEFILEPARSER_H

#include <QStringList>
#include <QVector>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QList>
#include <QMap>

namespace LangmuirCore {

/**
 * @brief A class to parse the PBS_NODEFILE and PBS_GPUFILE
 */
class NodeFileParser : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief create NodeFileParser
     * @param nodefile path to NODEFILE
     * @param gpufile path to GPUFILE
     * @param parent QObject this belongs to
     */
    explicit NodeFileParser(const QString& nodefile="", const QString& gpufile="", QObject *parent = 0);

    /**
     * @brief aquire the paths of the GPUFILE and the NODEFILE and parse them
     * @param nodefile path to NODEFILE
     * @param gpufile path to GPUFILE
     *
     * If nodefile is empty the enviroment variable PBS_NODEFILE is used.
     * If gpufile is empty, the enviroment variable PBS_GPUFILE is used.
     * If both paths remain empty, then setDefault() is used.
     */
    void setPaths(const QString& nodefile="", const QString& gpufile="");

    /**
     * @brief set the name of this CPU
     * @param hostName host name string
     */
    void setHostName(const QString& hostName);

    /**
     * @brief set the name of this CPU using boost::asio::ip::hostname()
     */
    void setHostName();

    /**
     * @brief set the default based on QThreadPool and hostname
     */
    void setDefault();

    /**
     * @brief add cpu to records
     * @param name name of cpu
     * @param cores number of cores
     * @param gpus list of GPU ids
     */
    void createNode(const QString &name, int cores=0, QList<int> gpus = QList<int>());

    /**
     * @brief clear the records
     */
    void clear();

    /**
     * @brief operator overload for QDebug
     */
    friend QDebug operator<<(QDebug dbg,const NodeFileParser& nfp);

    /**
     * @brief get the number of processes on all hosts
     */
    int numProc();

    /**
     * @brief get the number of processes on host
     * @param name hostname
     */
    int numProc(const QString& name);

    /**
     * @brief get the number of processes on all hosts
     */
    const QMap<QString,int>& procs();

    /**
     * @brief get the number of gpus on all hosts
     */
    int numGPUS();

    /**
     * @brief get the number of gpus on host
     * @param name hostname
     */
    int numGPUs(const QString& name);

    /**
     * @brief get the ith gpu id on host
     * @param name hostname
     * @param i index
     */
    int GPUid(const QString &name, int i);

    /**
     * @brief get the number of gpus on host
     * @param name hostname
     */
    const QList<int>& gpus(const QString& name);

    /**
     * @brief get the number of hosts
     */
    int numCPUs();

    /**
     * @brief get the hostnames
     */
    const QStringList& cpus();

    /**
     * @brief get the hostname of this cpu
     */
    const QString& hostName();

private:
    //! list of cpu names
    QStringList m_names;

    //! list of core counts per cpu
    QMap<QString,int> m_cores;

    //! list of gpu ids per cpu
    QMap<QString,QList<int> > m_gpus;

    //! path to NODEFILE
    QString m_nodefile;

    //! path to GPUFILE
    QString m_gpufile;

    //! hostname of this computer
    QString m_hostName;

    /**
     * parse NODEFILE or GPUFILE
     * @param filename name of file to parse
     * @param ignoreCores do not parse core information
     * @param ignoreGPUs do not parse gpu information
     * @return true if success
     */
    bool parse(QString &filename, bool ignoreCores = false, bool ignoreGPUs = true);
};

}

#endif // NODEFILEPARSER_H
