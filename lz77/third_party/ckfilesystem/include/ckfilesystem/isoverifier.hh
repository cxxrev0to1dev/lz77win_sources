/*
 * The ckFileSystem library provides file system functionality.
 * Copyright (C) 2006-2011 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include <vector>
#include <ckcore/types.hh>
#include <ckcore/canexstream.hh>

namespace ckfilesystem
{
    /**
     * @brief Class for verifications exceptions.
     */
    class VerificationException : public ckcore::Exception2
    {
    private:
        ckcore::tstring reference_;

    public:
        /**
         * Constructs a VerificationException object.
         * @param [in] msg The error message.
         * @param [in] ref Message describing the reference that contradicts
         *                 the behaviour causing the exception.
         */
        VerificationException(const ckcore::tstring &message,
                              const ckcore::tchar *reference) :
            ckcore::Exception2(message),reference_(reference)
        {
        }

        virtual ~VerificationException() throw() {};

        const ckcore::tstring &reference() const
        {
            return reference_;
        }
    };

    /**
     * @brief Class representing a volume descriptor set.
     */
    class IsoVolDescSet
    {
    private:
        tiso_voldesc_primary voldesc_primary_;
        std::vector<tiso_voldesc_suppl> voldesc_suppl_;
        std::vector<tiso_voldesc_part> voldesc_part_;
        std::vector<tiso_voldesc_bootrec> voldesc_bootrec_;
        std::vector<tiso_voldesc_setterm> voldesc_setterm_;

        void verify(const tiso_voldesc_datetime &voldesc_datetime,
                    const ckcore::tchar *label) const;
        void verify(tiso_voldesc_primary &voldesc_primary);
        void verify(const tiso_voldesc_suppl &voldesc_suppl,int index) const;

    public:
        IsoVolDescSet();

        void read(SectorInStream &in_stream);
        void verify();
    };

    class IsoVerifier
    {
    private:
        //tiso_voldesc_primary *voldesc_primary_;
        void read_vol_desc(SectorInStream &in_stream);

    public:
        IsoVerifier();
        ~IsoVerifier();

        void reset();
        void verify(SectorInStream &in_stream);

        static void verify_a_chars(const unsigned char *str,size_t len);
        static void verify_d_chars(const unsigned char *str,size_t len,bool allow_sep = false);
        static void verify_j_chars(const unsigned char *str,size_t len);

        static void read_a_chars(const unsigned char *source,size_t size,
                                 char *target);
        static void read_d_chars(const unsigned char *source,size_t size,
                                 char *target);
        static void read_j_chars(const unsigned char *source,size_t size,
                                 wchar_t *target);
    };
};
