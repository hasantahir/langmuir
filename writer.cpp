#include "writer.h"
#include "parameters.h"
#include "world.h"
#include "cubicgrid.h"
#include "chargeagent.h"
#include "fluxagent.h"
#include "openclhelper.h"

namespace Langmuir
{

XYZWriter::XYZWriter(World &world, const QString &name, OutputInfo::Options options, QObject *parent)
    : QObject(parent), m_world(world), m_stream(name,&m_world.parameters(),options,this)
{
}

FluxWriter::FluxWriter(World &world, const QString &name, OutputInfo::Options options, QObject *parent)
    : QObject(parent), m_world(world), m_stream(name,&m_world.parameters(),options,this)
{
    QList<FluxAgent *>& fluxAgents = m_world.fluxes();
    m_stream << qSetRealNumberPrecision(m_world.parameters().outputPrecision)
             << qSetFieldWidth(m_world.parameters().outputWidth)
             << right
             << scientific;
    m_stream << "simulation:time";
    foreach(FluxAgent *flux, fluxAgents)
    {
        m_stream << QString("%1:attempt").arg(flux->objectName());
        m_stream << QString("%1:success").arg(flux->objectName());
    }
    m_stream << "electron:count"
             << "electron:percentage"
             << "electron:reached"
             << "hole:count"
             << "hole:percentage"
             << "hole:reached"
             << "real:time"
             << newline;
    m_stream.flush();
}

CarrierWriter::CarrierWriter(World &world, const QString &name, OutputInfo::Options options, QObject *parent)
    : QObject(parent), m_world(world), m_stream(name,&m_world.parameters(),options,this)
{
    m_stream << qSetRealNumberPrecision(m_world.parameters().outputPrecision)
             << qSetFieldWidth(m_world.parameters().outputWidth)
             << right
             << scientific
             << "type"
             << "x"
             << "y"
             << "z"
             << "s"
             << "*"
             << "lifetime"
             << "pathlength"
             << "time"
             << newline
             << flush;
}

void XYZWriter::write()
{
    int num = m_world.numElectronAgents() +
              m_world.numHoleAgents()     +
              m_world.numDefects()        +
              m_world.numTraps();

    m_stream << qSetFieldWidth(0)
             << num
             << newline;

    m_stream << m_world.parameters().currentStep
             << newline;

    m_stream << qSetRealNumberPrecision(m_world.parameters().outputPrecision)
             << qSetFieldWidth(0)
             << right
             << scientific;
    {
        Grid &grid = m_world.electronGrid();
        QList<ChargeAgent*> &charges = m_world.electrons();
        foreach( ChargeAgent* pCharge, charges)
        {
            ChargeAgent &charge = *pCharge;
            int site = charge.getCurrentSite();
            m_stream << 'E'                  << ' '
                     << grid.getIndexX(site) << ' '
                     << grid.getIndexY(site) << ' '
                     << grid.getIndexZ(site) << ' '
                     << site                 << ' '
                     << &charge              << ' '
                     << charge.lifetime()    << ' '
                     << charge.pathlength()  << '\n';
        }
    }
    {
        Grid &grid = m_world.holeGrid();
        QList<ChargeAgent*> &charges = m_world.holes();
        foreach( ChargeAgent* pCharge, charges)
        {
            ChargeAgent &charge = *pCharge;
            int site = charge.getCurrentSite();
            m_stream << 'H'                  << ' '
                     << grid.getIndexX(site) << ' '
                     << grid.getIndexY(site) << ' '
                     << grid.getIndexZ(site) << ' '
                     << site                 << ' '
                     << &charge              << ' '
                     << charge.lifetime()    << ' '
                     << charge.pathlength()  << '\n';
        }
    }
    {
        Grid &grid = m_world.electronGrid();
        QList<int> &ids = m_world.defectSiteIDs();
        for (int i = 0; i < ids.size(); i++)
        {
            int site = ids[i];
            m_stream << 'D'                  << ' '
                     << grid.getIndexX(site) << ' '
                     << grid.getIndexY(site) << ' '
                     << grid.getIndexZ(site) << ' '
                     << site                 << ' '
                     << i                    << '\n';
        }
    }
    {
        Grid &grid = m_world.electronGrid();
        QList<int> &ids = m_world.trapSiteIDs();
        for (int i = 0; i < ids.size(); i++)
        {
            int site = ids[i];
            m_stream << 'T'                  << ' '
                     << grid.getIndexX(site) << ' '
                     << grid.getIndexY(site) << ' '
                     << -1                   << ' '
                     << site                 << ' '
                     << i                    << '\n';
        }
    }
}

void FluxWriter::write()
{
    QDateTime now = QDateTime::currentDateTime();
    QList<FluxAgent *>& fluxAgents =  m_world.fluxes();
    m_stream << m_world.parameters().currentStep;
    foreach(FluxAgent *flux, fluxAgents)
    {
        m_stream << flux->attempts() << flux->successes();
    }
    m_stream << m_world.numElectronAgents()
             << m_world.percentElectronAgents()
             << m_world.reachedElectronAgents()
             << m_world.numHoleAgents()
             << m_world.percentHoleAgents()
             << m_world.reachedHoleAgents()
             << m_world.parameters().simulationStart.msecsTo(now)
             << newline;
    m_stream.flush();
}

void CarrierWriter::write(ChargeAgent &charge)
{
    Grid &grid = charge.getGrid();
    int site = charge.getCurrentSite();
    m_stream << charge.getType()
             << grid.getIndexX(site)
             << grid.getIndexY(site)
             << grid.getIndexZ(site)
             << site
             << &charge
             << charge.lifetime()
             << charge.pathlength()
             << m_world.parameters().currentStep
             << newline;
}

GridImage::GridImage(World &world, QColor bg, QObject *parent)
    : QObject(parent), m_world(world)
{
    int xsize = m_world.parameters().gridX;
    int ysize = m_world.parameters().gridY;
    m_image = QImage(xsize,ysize,QImage::Format_ARGB32_Premultiplied);
    m_image.fill(0);
    m_painter.begin(&m_image);
    m_painter.scale(1.0,-1.0);
    m_painter.translate(0.0,-ysize);
    m_painter.setWindow(QRect(0,0,xsize,ysize));
    m_painter.fillRect(QRect(0,0,xsize,ysize),bg);
}

void GridImage::save(QString name, int scale)
{
    OutputInfo info(name, &m_world.parameters());
    m_painter.end();
    m_image = m_image.scaled(scale*m_image.width(),scale*m_image.height(),Qt::KeepAspectRatioByExpanding);
    m_image.save(info.absoluteFilePath(),"png",100);
}

void GridImage::drawSites( QList<int> &sites, QColor color, int layer )
{
    if (sites.size()<=0)return;
    Grid &grid = m_world.electronGrid();
    m_painter.setPen(color);
    for(int i = 0; i < sites.size(); i++)
    {
        int ndx = sites[i];
        if(grid.getIndexZ(ndx)== layer)
        {
            m_painter.drawPoint(QPoint(grid.getIndexX(ndx),grid.getIndexY(ndx)));
        }
    }
}

void GridImage::drawCharges( QList<ChargeAgent *> &charges,QColor color, int layer )
{
    if (charges.size()<=0)return;
    Grid &grid = m_world.electronGrid();
    m_painter.setPen(color);
    for(int i = 0; i < charges.size(); i++)
    {
        int ndx = charges[i]->getCurrentSite();
        if(grid.getIndexZ(ndx)== layer)
        {
            m_painter.drawPoint(QPoint(grid.getIndexX(ndx),grid.getIndexY(ndx)));
        }
    }
}

LoggerOn::LoggerOn(World &world, QObject *parent)
    : Logger(world,parent), m_world(world), m_xyzWriter(0), m_fluxWriter(0), m_carrierWriter(0)
{
    initialize();
}

void LoggerOn::initialize()
{
    if (m_world.parameters().outputXyz)
    {
        m_xyzWriter = new XYZWriter(m_world,"%path/%stub-%sim.xyz",0,this);
    }

    if (m_world.parameters().outputIdsOnDelete)
    {
        m_carrierWriter = new CarrierWriter(m_world,"%path/%stub-%sim-carriers.dat",0,this);
    }

    m_fluxWriter = new FluxWriter(m_world,"%path/%stub-%sim.dat",0,this);
}

void LoggerOn::saveGridPotential(const QString& name)
{
    Grid &grid = m_world.electronGrid();

    OutputStream stream(name,&m_world.parameters(),0,this);

    stream << qSetRealNumberPrecision(m_world.parameters().outputPrecision)
           << qSetFieldWidth(m_world.parameters().outputWidth)
           << right
           << scientific;

    stream << "s"
           << "x"
           << "y"
           << "z"
           << "e"
           << "h"
           << newline;

    for(int i = 0; i < m_world.electronGrid().xSize(); i++)
    {
        for(int j = 0; j < m_world.electronGrid().ySize(); j++)
        {
            for(int k = 0; k < m_world.electronGrid().zSize(); k++)
            {
                int si = grid.getIndexS(i,j,k);
                stream << si
                       << i
                       << j
                       << k
                       << m_world.electronGrid().potential(si)
                       << m_world.holeGrid().potential(si)
                       << newline;
            }
        }
    }
    stream << flush;
}

void LoggerOn::saveCoulombEnergy(const QString& name)
{
    Grid &grid = m_world.electronGrid();
    OpenClHelper &openCL = m_world.opencl();

    OutputStream stream(name,&m_world.parameters(),0,this);

    stream << qSetRealNumberPrecision(m_world.parameters().outputPrecision)
           << qSetFieldWidth(m_world.parameters().outputWidth)
           << right
           << scientific;

    stream << "s"
           << "x"
           << "y"
           << "z"
           << "v"
           << newline;

    for(int i = 0; i < grid.xSize(); i++)
    {
        for(int j = 0; j < grid.ySize(); j++)
        {
            for(int k = 0; k < grid.zSize(); k++)
            {
                int si = grid.getIndexS(i,j,k);
                stream << si
                       << i
                       << j
                       << k
                       << openCL.getOutputHost(si)
                       << newline;
            }
        }
    }
    stream << flush;
}

void LoggerOn::reportFluxStream()
{
    if (m_fluxWriter) m_fluxWriter->write();
}

void LoggerOn::reportXYZStream()
{
    if (m_xyzWriter) m_xyzWriter->write();
}

void LoggerOn::reportCarrier(ChargeAgent &charge)
{
    if (m_carrierWriter) m_carrierWriter->write(charge);
}

void LoggerOn::saveTrapImage(const QString& name)
{
    QList<int>& siteIDs = m_world.trapSiteIDs();
    if(siteIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawSites(siteIDs,Qt::green,0);
    image.save(name);
}

void LoggerOn::saveDefectImage(const QString& name)
{
    QList<int>& siteIDs = m_world.defectSiteIDs();
    if(siteIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawSites(siteIDs,Qt::cyan,0);
    image.save(name);
}

void LoggerOn::saveElectronImage(const QString& name)
{
    QList<ChargeAgent*>& chargeIDs = m_world.electrons();
    if(chargeIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawCharges(chargeIDs,Qt::red,0);
    image.save(name);
}

void LoggerOn::saveHoleImage(const QString& name)
{
    QList<ChargeAgent*>& chargeIDs = m_world.holes();
    if(chargeIDs.size()==0)return;
    GridImage image(m_world,Qt::white,this);
    image.drawCharges(chargeIDs,Qt::blue,0);
    image.save(name);
}

void LoggerOn::saveImage(const QString& name)
{
    QList<int>& traps = m_world.trapSiteIDs();
    QList<int>& defects = m_world.defectSiteIDs();
    QList<ChargeAgent*>& electrons = m_world.electrons();
    QList<ChargeAgent*>& holes = m_world.holes();
    GridImage image(m_world,Qt::white,this);
    if (traps.size()>0) { image.drawSites(traps,Qt::green,0); }
    if (defects.size()>0) { image.drawSites(defects,Qt::cyan,0); }
    if (electrons.size()>0) { image.drawCharges(electrons,Qt::red,0); }
    if (holes.size()>0) { image.drawCharges(holes,Qt::blue,0); }
    image.save(name);
}

}