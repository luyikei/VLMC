/*****************************************************************************
 * SettingValue.h: A setting value that can broadcast its changes
 *****************************************************************************
 * Copyright (C) 2008-2016 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifndef SETTINGVALUE_H
#define SETTINGVALUE_H

#include <QObject>
#include <QVariant>

/**
 * 'class SettingValue
 *
 * \brief represent a setting value
 *
 */
class   SettingValue : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( SettingValue )
    public:
        enum    Type
        {
            Int,
            Double,
            String,
            Bool,
            Language,
            KeyboardShortcut,
            Path,
            ByteArray, // For now this is only for private variables, and is not expected to be used at any time
            //For effect engine settings:
            List,
            Map,
            Hash,
            Color,
            Position,
        };
        enum    Flag
        {
            Nothing         = 0,
            /// If this flag is used, then the variable should not be shown in the config widgets.
            Private         = 1 << 0,
            Password        = 1 << 1,
            Clamped         = 1 << 2, ///< When used, the m_min and m_max will be used
            EightMultiple   = 1 << 3, ///< Forces the value to be a multiple of 8
            NotEmpty        = 1 << 4, ///< Forces the value not to be empty (likely to be used only with Strings)
            Runtime         = 1 << 5, ///< Defines a variable that is not meant to be saved
            Folders         = 1 << 6, ///< Specifies that a setting should only contain folders.
        };
        Q_DECLARE_FLAGS( Flags, Flag )
        /**
         *  \brief      Constructs a setting value with its default value and description
         *
         *  \param      key             The parameter key - ie. the value used
         *                              to fetch this setting in SettingsManager
         *  \param      name            The name of this setting. Not his key.
         *                              This is used when generating the settings GUI
         *                              and will be translated.
         *  \param      defaultValue    The setting default value.
         *  \param      desc            The setting description
         */
        SettingValue( const QString& key, Type type, const QVariant& defaultValue, const char* name,
                      const char* desc, Flags flags = Nothing );

        /**
         * \brief setter for the m_val member
         * \param   val the value wich will be affected to m_val
         */

        virtual void    set( const QVariant& val );

        /**
         * \brief getter for the m_val member
         */
        virtual const QVariant& get(); //Not const to avoid a mess with EffectSettingValue.
        /**
         *  \return The setting's description
         */
        const char      *description() const;
        /**
         *   \brief     Set the setting to its default value.
         */
        void            restoreDefault();

        /**
         * @brief key   Returns the key for this setting
         *
         * @warning     This is NOT the name to be used when generating settings GUI
         */
        const QString&  key() const;

        /**
         * @brief name  The name of this setting.
         *
         * @warning     This is NOT the key to be used when fetching the setting.
         */
        const char      *name() const;
        Type            type() const;
        Flags           flags() const;

        void            setLimits( const QVariant& min, const QVariant& max );
        const QVariant& min() const;
        const QVariant& max() const;

    protected:
        /**
         * \brief the QVariant containingthe value of the settings
         */
        QVariant        m_val;
        QVariant        m_defaultVal;
        const QString   m_key;
        const char*     m_name;
        const char*     m_desc;
        Type            m_type;
        Flags           m_flags;
        QVariant        m_min;
        QVariant        m_max;
        bool            m_initLoad;
    signals:
        /**
         * \brief This signal is emmited while the m_val
         *        member have been modified
         */
        void        changed( const QVariant& );
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SettingValue::Flags)

#endif // SETTINGVALUE_H
