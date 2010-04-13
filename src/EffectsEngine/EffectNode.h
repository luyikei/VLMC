/*****************************************************************************
 * EffectNode.h: Node of the patch
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Vincent Carrubba <cyberbouba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef EFFECTNODE_H_
#define EFFECTNODE_H_

#include "EffectNodeFactory.h"
#include "IEffectNode.h"
#include "InSlot.hpp"
#include "OutSlot.hpp"
#include "SemanticObjectManager.hpp"
#include "SimpleObjectsReferencer.hpp"

#include <QQueue>
#include <QtGlobal>

class   IEffectPlugin;
class   LightVideoFrame;

class   QReadLocker;
class   QReadWriteLock;
class   QString;
class   QWriteLocker;
class   QObject;

class	EffectNode : public IEffectNode
{
    ////
    ////
    ////
    ////
    //// METHODS
    ////
    ////
    ////
    ////

 public:

    // ================================================================= CTORS, DTOR & INIT ========================================================================

    ~EffectNode();

    EffectNode( IEffectPlugin* plugin );
    EffectNode();

    // ================================================================= INFOS ========================================================================

    void        setTypeId( quint32 typeId );
    void        setTypeName( const QString & typeName );
    void        setInstanceId( quint32 instanceId );
    void        setInstanceName( const QString & instanceName );

    quint32             getTypeId( void ) const;
    const QString &     getTypeName( void ) const;
    quint32             getInstanceId( void ) const;
    const QString &     getInstanceName( void ) const;

    bool                isAnEmptyNode( void ) const;

    // ================================================================= RENDER ========================================================================

    void        render( void );
    void        renderSubNodes( void );
    void        transmitDatasFromInputsToInternalsOutputs( void );
    void        transmitDatasFromInternalsInputsToOutputs( void );
    void        resetAllChildsNodesVisitState( void );
    void        setVisited( void );
    void        resetVisitState( void );
    bool        wasItVisited( void ) const;

    // ================================================================= GET WIDGET ========================================================================

    //     QWidget*            getWidget( void );

    // ================================================================= INTERNAL PLUGIN ========================================================================

    IEffectPlugin*       getInternalPlugin( void );

    // ================================================================= PARENT NODE ========================================================================

    void                setFather( EffectNode* father );
    IEffectNode*        getFather( void ) const;
    EffectNode*         getPrivateFather( void ) const;

    // ================================================================= ROOT NODES ========================================================================

    static bool                createRootNode( const QString & rootNodeName );
    static bool                deleteRootNode( const QString & rootNodeName );
    static EffectNode*         getRootNode( const QString & rootNodeName );

    // ================================================================= CHILD NODES ========================================================================

    // ------------------- CHILDS TYPES INFORMATIONS -------------------

    QList<QString>              getChildsTypesNamesList( void ) const;
    QList<quint32>              getChildsTypesIdsList( void ) const;

    const QString               getChildTypeNameByTypeId( quint32 typeId ) const;
    quint32                     getChildTypeIdByTypeName( const QString & typeName ) const;

    // ------------------- CHILDS INFORMATIONS -------------------

    QList<QString>              getChildsNamesList( void ) const;
    QList<quint32>              getChildsIdsList( void ) const;

    const QString               getChildNameByChildId( quint32 childId ) const;
    quint32                     getChildIdByChildName( const QString & childName ) const;

    // ------------------- CREATE AND DELETE CHILDS -------------------

    bool        createEmptyChild( void );
    bool        createEmptyChild( const QString & childName );

    bool        createChild( quint32 typeId );
    bool        createChild( const QString & typeName );

    bool        deleteChild( quint32 childId );
    bool        deleteChild( const QString & childName );

    // ------------------- GETTING CHILDS -------------------

    EffectNode* getChild( quint32 childId ) const;
    EffectNode* getChild( const QString & childName ) const;

    QList<EffectNode*>  getChildsList( void ) const;

    // ================================================================= SLOTS ========================================================================

    //------------------------------------------------------------//
    //                 MANAGING STATICS SLOTS                     //
    //------------------------------------------------------------//

    // -------------- CREATE --------------

    void		createStaticVideoInput( void );
    void		createStaticVideoOutput( void );
    //     void		createStaticAudioInput( void );
    //     void		createStaticAudioOutput( void );
    //     void		createStaticControlInput( void );
    //     void		createStaticControlOutput( void );

    void		createStaticVideoInput( const QString & name );
    void		createStaticVideoOutput( const QString & name );
    //     void		createStaticAudioInput( QByteArray const & name );
    //     void		createStaticAudioOutput( QByteArray const & name );
    //     void		createStaticControlInput( QByteArray const & name );
    //     void		createStaticControlOutput( QByteArray const & name );

    // -------------- REMOVE --------------

    bool		removeStaticVideoInput( const QString & name );
    bool		removeStaticVideoOutput( const QString & name );
    //     bool		removeStaticAudioInput( QByteArray const & name );
    //     bool		removeStaticAudioOutput( QByteArray const & name );
    //     bool		removeStaticControlInput( QByteArray const & name );
    //     bool		removeStaticControlOutput( QByteArray const & name );

    bool		removeStaticVideoInput( quint32 id );
    bool		removeStaticVideoOutput( quint32 id );
    //     bool		removeStaticAudioInput( quint32 id );
    //     bool		removeStaticAudioOutput( quint32 id );
    //     bool		removeStaticControlInput( quint32 id );
    //     bool		removeStaticControlOutput( quint32 id );

    // -------------- GET --------------

    InSlot<LightVideoFrame>*		getStaticVideoInput( const QString & name ) const;
    OutSlot<LightVideoFrame>*	        getStaticVideoOutput( const QString & name ) const;
    //     InSlot<AudioSoundSample>*		getStaticAudioInput( QByteArray const & name ) const;
    //     OutSlot<AudioSoundSample>*		getStaticAudioOutput( QByteArray const & name ) const;
    //     InSlot<qreal>*		getStaticControlInput( QByteArray const & name ) const;
    //     OutSlot<qreal>* 		getStaticControlOutput( QByteArray const & name ) const;

    InSlot<LightVideoFrame>*		getStaticVideoInput( quint32 id ) const;
    OutSlot<LightVideoFrame>*		getStaticVideoOutput( quint32 id ) const;
    //  InSlot<AudioSoundSample>*		getStaticAudioInput( quint32 id ) const;
    //  OutSlot<AudioSoundSample>*		getStaticAudioOutput( quint32 id ) const;
    //  InSlot<qreal>*		getStaticControlInput( quint32 id ) const;
    //  OutSlot<qreal>*		getStaticControlOutput( quint32 id ) const;

    QList<InSlot<LightVideoFrame>*>		getStaticsVideosInputsList( void ) const;
    QList<OutSlot<LightVideoFrame>*>		getStaticsVideosOutputsList( void ) const;
    //  QList<InSlot<AudioSoundSample>*>		getStaticsAudiosInputsList( void ) const;
    //     QList<OutSlot<AudioSoundSample>*>		getStaticsAudiosOutputsList( void ) const;
    //     QList<InSlot<qreal>*>		getStaticsControlsInputsList( void ) const;
    //     QList<OutSlot<qreal>*>		getStaticsControlsOutputsList( void ) const;

    // -------------- GET INTERNALS ( JUST FOR EMPTY NODES) --------------

    InSlot<LightVideoFrame>*		getInternalStaticVideoInput( const QString & name ) const;
    OutSlot<LightVideoFrame>*	        getInternalStaticVideoOutput( const QString & name ) const;
    //     InSlot<AudioSoundSample>*		getStaticAudioInput( QByteArray const & name ) const;
    //     OutSlot<AudioSoundSample>*		getStaticAudioOutput( QByteArray const & name ) const;
    //     InSlot<qreal>*		getStaticControlInput( QByteArray const & name ) const;
    //     OutSlot<qreal>* 		getStaticControlOutput( QByteArray const & name ) const;

    InSlot<LightVideoFrame>*		getInternalStaticVideoInput( quint32 id ) const;
    OutSlot<LightVideoFrame>*		getInternalStaticVideoOutput( quint32 id ) const;
    //  InSlot<AudioSoundSample>*		getStaticAudioInput( quint32 id ) const;
    //  OutSlot<AudioSoundSample>*		getStaticAudioOutput( quint32 id ) const;
    //  InSlot<qreal>*		getStaticControlInput( quint32 id ) const;
    //  OutSlot<qreal>*		getStaticControlOutput( quint32 id ) const;

    QList<InSlot<LightVideoFrame>*>		getInternalsStaticsVideosInputsList( void ) const;
    QList<OutSlot<LightVideoFrame>*>		getInternalsStaticsVideosOutputsList( void ) const;
    //  QList<InSlot<AudioSoundSample>*>		getStaticsAudiosInputsList( void ) const;
    //     QList<OutSlot<AudioSoundSample>*>		getStaticsAudiosOutputsList( void ) const;
    //     QList<InSlot<qreal>*>		getStaticsControlsInputsList( void ) const;
    //     QList<OutSlot<qreal>*>		getStaticsControlsOutputsList( void ) const;

    // -------------- GET INFOS ON SLOTS --------------

    QList<QString>	getStaticsVideosInputsNamesList( void ) const;
    QList<QString>	getStaticsVideosOutputsNamesList( void ) const;
    //     QList<QByteArray> const &	getStaticAudiosInputsNameList( void ) const;
    //     QList<QByteArray> const &	getStaticAudiosOutputsNameList( void ) const;
    //     QList<QByteArray> const &	getStaticControlsInputsNameList( void ) const;
    //     QList<QByteArray> const &	getStaticControlsOutputsNameList( void ) const;

    QList<quint32>	getStaticsVideosInputsIdsList( void ) const;
    QList<quint32>	getStaticsVideosOutputsIdsList( void ) const;
    //     QList<QByteArray> const &	getStaticAudiosInputsIdList( void ) const;
    //     QList<QByteArray> const &	getStaticAudiosOutputsIdList( void ) const;
    //     QList<QByteArray> const &	getStaticControlsInputsIdList( void ) const;
    //     QList<QByteArray> const &	getStaticControlsOutputsIdList( void ) const;

    const QString        getStaticVideoInputNameById( quint32 id ) const;
    const QString        getStaticVideoOutputNameById( quint32 id ) const;
    //     QByteArray const &          getStaticAudioInputNameById( quint32 const id ) const;
    //     QByteArray const &          getStaticAudioOutputNameById( quint32 const id ) const;
    //     QByteArray const &          getStaticControlInputNameById( quint32 const id ) const;
    //     QByteArray const &          getStaticControlOutputNameById( quint32 const id ) const;

    quint32              getStaticVideoInputIdByName( const QString & name ) const;
    quint32              getStaticVideoOutputIdByName( const QString & name ) const;
    //     quint32                     getStaticAudioInputIdByName( QByteArray const & name ) const;
    //     quint32                     getStaticAudioOutputIdByName( QByteArray const & name ) const;
    //     quint32                     getStaticControlInputIdByName( QByteArray const & name ) const;
    //     quint32                     getStaticControlOutputIdByName( QByteArray const & name ) const;

    quint32              getNBStaticsVideosInputs( void ) const;
    quint32              getNBStaticsVideosOutputs( void ) const;
    //     quint32                     getNBStaticAudiosIntputs( void ) const;
    //     quint32                     getNBStaticAudiosOutputs( void ) const;
    //     quint32                     getNBStaticControlsInputs( void ) const;
    //     quint32                     getNBStaticControlsOutputs( void ) const;

    //------------------------------------------------------------//
    //                 MANAGING DYNAMICS SLOTS                    //
    //------------------------------------------------------------//

    // -------------- ENABLE/DISABLE --------------

    //     void		enableDynamicVideoInput( void );
    //     void		enableDynamicVideoOutput( void );
    //     void		enableDynamicAudioInput( void );
    //     void		enableDynamicAudioOutput( void );
    //     void		enableDynamicControlInput( void );
    //     void		enableDynamicControlOutput( void );

    //     void		disableDynamicVideoInput( void );
    //     void		disableDynamicVideoOutput( void );
    //     void		disableDynamicAudioInput( void );
    //     void		disableDynamicAudioOutput( void );
    //     void		disableDynamicControlInput( void );
    //     void		disableDynamicControlOutput( void );

    // -------------- GET SLOTS --------------

    //     QList<OutSlot<LightVideoFrame>*>    getDynamicVideoOutputList( void ) const;
    //     QList<InSlot<LightVideoFrame>*>     getDynamicVideoInputList( void ) const;

    //     QList<OutSlot<AudioSoundSample>*>    getDynamicAudioOutputList( void ) const;
    //     QList<InSlot<AudioSoundSample>*>     getDynamicAudioInputList( void ) const;

    //     QList<OutSlot<qreal>*>    getDynamicControlOutputList( void ) const;
    //     QList<InSlot<qreal>*>     getDynamicControlInputList( void ) const;


    //     QList<OutSlot<LightVideoFrame>*>    getDynamicVideoOutputById( quint32 id ) const;
    //     QList<InSlot<LightVideoFrame>*>     getDynamicVideoInputById( quint32 id ) const;

    //     QList<OutSlot<AudioSoundSample>*>    getDynamicAudioOutputById( quint32 id ) const;
    //     QList<InSlot<AudioSoundSample>*>     getDynamicAudioInputById( quint32 id ) const;

    //     QList<OutSlot<qreal>*>    getDynamicControlOutputById( quint32 ) const;
    //     QList<InSlot<qreal>*>     getDynamicControlInputById( quint32 ) const;

    // -------------- GET INFOS ON SLOTS --------------

    //     quint32             getNBDynamicVideosInputs( void ) const;
    //     quint32             getNBDynamicVideosOutputs( void ) const;
    //     quint32             getNBDynamicAudiosIntputs( void ) const;
    //     quint32             getNBDynamicAudiosOutputs( void ) const;
    //     quint32             getNBDynamicControlsInputs( void ) const;
    //     quint32             getNBDynamicControlsOutputs( void ) const;

    //     bool                areDynamicVideosInputsEnabled( void ) const;
    //     bool                areDynamicVideosOutputsEnabled( void ) const;
    //     bool                areDynamicAudiosIntputsEnabled( void ) const;
    //     bool                areDynamicAudiosOutputsEnabled( void ) const;
    //     bool                areDynamicControlsInputsEnabled( void ) const;
    //     bool                areDynamicControlsOutputsEnabled( void ) const;

    //--------------------------------------------------------------------//
    //                            CONNECTION                              //
    //--------------------------------------------------------------------//

 private:

    bool        primitiveConnection( quint32 outId,
                                     const QString &outName,
                                     bool outInternal,
                                     quint32 inId,
                                     const QString &inName,
                                     bool inInternal,
                                     quint32 nodeId,
                                     const QString &nodeName );
 public:

    /**
     * Connect brother to brother
     */

    bool        connectBrotherToBrother( const QString & outName, const QString & nodeName, const QString & inName );
    bool        connectBrotherToBrother( const QString & outName, const QString & nodeName, quint32 inId );
    bool        connectBrotherToBrother( const QString & outName, quint32 nodeId, const QString & inName );
    bool        connectBrotherToBrother( const QString & outName, quint32 nodeId, quint32 inId );
    bool        connectBrotherToBrother( quint32 outId, const QString & nodeName, const QString & inName );
    bool        connectBrotherToBrother( quint32 outId, const QString & nodeName, quint32 inId );
    bool        connectBrotherToBrother( quint32 outId, quint32 nodeId, const QString & inName );
    bool        connectBrotherToBrother( quint32 outId, quint32 nodeId, quint32 inId );

    /**
     * Connect child to parent
     */

    bool        connectChildToParent( const QString & outName, const QString & inName );
    bool        connectChildToParent( const QString & outName, quint32 inId );
    bool        connectChildToParent( quint32 outId, const QString & inName );
    bool        connectChildToParent( quint32 outId, quint32 inId );

    /**
     * Connect parent to child
     */

    bool        connectParentToChild( const QString & outName, const QString & nodeName, const QString & inName );
    bool        connectParentToChild( const QString & outName, const QString & nodeName, quint32 inId );
    bool        connectParentToChild( quint32 outId, const QString & nodeName, const QString & inName );
    bool        connectParentToChild( quint32 outId, const QString & nodeName, quint32 inId );

    bool        connectParentToChild( const QString & outName, quint32 nodeId, const QString & inName );
    bool        connectParentToChild( const QString & outName, quint32 nodeId, quint32 inId );
    bool        connectParentToChild( quint32 outId, quint32 nodeId, const QString & inName );
    bool        connectParentToChild( quint32 outId, quint32 nodeId, quint32 inId );

    /**
     * Connect internal bridge
     */

    bool        connectInternalBridge( const QString & outName, const QString & inName );
    bool        connectInternalBridge( const QString & outName, quint32 inId );
    bool        connectInternalBridge( quint32 outId, const QString & inName );
    bool        connectInternalBridge( quint32 outId, quint32 inId );

    //-------------------------------------------------------------------------//
    //                             DISCONNECTIONS                              //
    //-------------------------------------------------------------------------//

 private:

    bool        primitiveDisconnection( quint32 nodeId, const QString & nodeName, bool internal );

 public:

    /**
     * External slot disconnection
     */

    bool        disconnectStaticVideoOutput( quint32 nodeId );
    bool        disconnectStaticVideoOutput( const QString & nodeName );

    /**
     * Internal slot disconnection
     */

    bool         disconnectInternalStaticVideoOutput( quint32 nodeId );
    bool         disconnectInternalStaticVideoOutput( const QString & nodeName );

 private:

    //-------------------------------------------------------------------------//
    //                     CONNECTED SLOTS MAP MANAGEMENT                      //
    //-------------------------------------------------------------------------//

    bool             referenceStaticVideoInputAsConnected( InSlot<LightVideoFrame>* in );
    bool             referenceInternalStaticVideoOutputAsConnected( OutSlot<LightVideoFrame>* out );
    bool             referenceStaticVideoOutputAsConnected( OutSlot<LightVideoFrame>* out );
    bool             referenceInternalStaticVideoInputAsConnected( InSlot<LightVideoFrame>* in );

    bool             dereferenceStaticVideoInputAsConnected( quint32 inId );
    bool             dereferenceInternalStaticVideoOutputAsConnected( quint32 outId );
    bool             dereferenceStaticVideoOutputAsConnected( quint32 outId );
    bool             dereferenceInternalStaticVideoInputAsConnected( quint32 inId );

    QList<InSlot<LightVideoFrame>*>  getConnectedStaticsVideosInputsList( void ) const;
    QList<OutSlot<LightVideoFrame>*> getConnectedInternalsStaticsVideosOutputsList( void ) const;
    QList<OutSlot<LightVideoFrame>*> getConnectedStaticsVideosOutputsList( void ) const;
    QList<InSlot<LightVideoFrame>*>  getConnectedInternalsStaticsVideosInputsList( void ) const;

    quint32                          getNBConnectedStaticsVideosInputs( void ) const;
    quint32                          getNBConnectedInternalsStaticsVideosOutputs( void ) const;
    quint32                          getNBConnectedStaticsVideosOutputs( void ) const;
    quint32                          getNBConnectedInternalsStaticsVideosInputs( void ) const;

    ////
    ////
    ////
    ////
    //// ATTRIBUTES
    ////
    ////
    ////
    ////

 private:

    static EffectNodeFactory            s_renf;
    static QReadWriteLock               s_srwl;

 private:

    mutable QReadWriteLock              m_rwl;
    EffectNodeFactory                   m_enf;
    EffectNode*                         m_father;
    IEffectPlugin*                      m_plugin;
    bool                                m_visited;

    //
    //
    //
    // EFFECT TYPE AND EFFECT INSTANCE INFORMATIONS
    //
    //
    //

    quint32                             m_typeId;
    QString                             m_typeName;
    quint32                             m_instanceId;
    QString                             m_instanceName;

    //
    //
    // SLOTS
    //
    //

    //
    // STATIC SLOTS
    //

    // VIDEOS SLOTS

    SemanticObjectManager< InSlot<LightVideoFrame> >	m_staticVideosInputs;
    SemanticObjectManager< OutSlot<LightVideoFrame> >	m_internalsStaticVideosOutputs;
    SemanticObjectManager< OutSlot<LightVideoFrame> >	m_staticVideosOutputs;
    SemanticObjectManager< InSlot<LightVideoFrame> >	m_internalsStaticVideosInputs;

    SimpleObjectsReferencer<InSlot<LightVideoFrame> >   m_connectedStaticVideosInputs;
    SimpleObjectsReferencer<OutSlot<LightVideoFrame> >  m_connectedInternalsStaticVideosOutputs;
    SimpleObjectsReferencer<OutSlot<LightVideoFrame> >  m_connectedStaticVideosOutputs;
    SimpleObjectsReferencer<InSlot<LightVideoFrame> >   m_connectedInternalsStaticVideosInputs;

    // AUDIOS SLOTS

    //     SemanticObjectManager< InSlot<AudioSoundSample> >  m_staticAudiosInputs;
    //     SemanticObjectManager< OutSlot<AudioSoundSample> > m_staticAudiosOutputs;

    // CONTROLS SLOTS

    //     SemanticObjectManager< InSlot<qreal> >	m_staticControlsInputs;
    //     SemanticObjectManager< OutSlot<qreal> >	m_staticControlsOutputs;

    //
    // DYNAMIC SLOTS
    //

    // VIDEOS SLOTS

    //     QList< InSlot<LightVideoFrame>* >                    m_dynamicVideosInputs;
    //     QList< OutSlot<LightVideoFrame>* >                   m_dynamicVideosOutputs;

    // AUDIOS SLOTS

    // QList< InSlot<AudioSoundSample>* >                                m_dynamicAudiosInputs;
    // QList< OutSlot<AudioSoundSample>* >                               m_dynamicAudiosOutputs;



    // CONTROLS SLOTS

    //     QList< InSlot<qreal>* >                              m_dynamicControlsInputs;
    //     QList< OutSlot<qreal>* >                             m_dynamicControlsOutputs;

    //
    // DYNAMICS SLOTS INFOS
    //

    // VIDEOS SLOTS

    //     bool                                        m_enableDynamicVideosInputs;
    //     bool                                        m_enableDynamicVideosOutputs;

    // AUDIOS SLOTS

    //     bool                                        m_enableDynamicAudiosInputs;
    //     bool                                        m_enableDynamicAudiossOutputs;

    // CONTROLS SLOTS

    //     bool                                        m_enableDynamicControlInputs;
    //     bool                                        m_enableDynamicControlOutputs;
};

#endif // EFFECTNODE_H_
