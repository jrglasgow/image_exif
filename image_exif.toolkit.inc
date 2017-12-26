<?php

/**
 * @file
 *
 * contain the image toolkit necessary callbacks
 */

 /**
 * @see image_gd_settings().
 */
function image_exif_settings() {
  module_load_include('inc', 'system', 'image.gd');
  $form =  image_gd_settings();
  $form['status']['#markup'] .= ' ' . t('EXIF Metadata Retention relies heavily on "GD2 image manipulation toolkit".');
  return $form;
}

/**
 * @see image_gd_settings_validate().
 */
function image_exif_settings_validate($form, &$form_state) {
  module_load_include('inc', 'system', 'image.gd');
  image_gd_settings_validate($form, $form_state);
}

/**
 * @see image_gd_resize().
 */
function image_exif_resize(stdClass $image, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_resize($image, $width, $height);
}

/**
 * @see image_gd_rotate().
 */
function image_exif_rotate(stdClass $image, $degrees, $background = NULL) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_rotate($image, $degrees, $background = NULL);
}

/**
 * @see image_gd_crop().
 */
function image_exif_crop(stdClass $image, $x, $y, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_crop($image, $x, $y, $width, $height);
}

/**
 * @see image_gd_desaturate().
 */
function image_exif_desaturate(stdClass $image) {
  return image_gd_desaturate($image);
}

/**
 * @see image_gd_load().
 */
function image_exif_load(stdClass $image) {
  return image_gd_load($image);
}

/**
 * GD helper to write an image resource to a destination file.
 *
 * @param $image
 *   An image object.
 * @param $destination
 *   A string file URI or path where the image should be saved.
 * @return
 *   TRUE or FALSE, based on success.
 *
 * @see image_save()
 * @see image_gd_save().
 */
function image_exif_save(stdClass $image, $destination) {
  $scheme = file_uri_scheme($destination);
  // Work around lack of stream wrapper support in imagejpeg() and imagepng().
  if ($scheme && file_stream_wrapper_valid_scheme($scheme)) {
    // If destination is not local, save image to temporary local file.
    $local_wrappers = file_get_stream_wrappers(STREAM_WRAPPERS_LOCAL);
    if (!isset($local_wrappers[$scheme])) {
      $permanent_destination = $destination;
      $destination = drupal_tempnam('temporary://', 'gd_');
    }
    // Convert stream wrapper URI to normal path.
    $destination = drupal_realpath($destination);
  }

  $extension = str_replace('jpg', 'jpeg', $image->info['extension']);
  $function = 'image' . $extension;
  if (!function_exists($function)) {
    return FALSE;
  }
  if ($extension == 'jpeg') {
    $success = $function($image->resource, $destination, variable_get('image_jpeg_quality', 75));
    image_exif_save_metadata($image, $destination);
  }
  else {
    // Always save PNG images with full transparency.
    if ($extension == 'png') {
      imagealphablending($image->resource, FALSE);
      imagesavealpha($image->resource, TRUE);
    }
    $success = $function($image->resource, $destination);
  }
  // Move temporary local file to remote destination.
  if (isset($permanent_destination) && $success) {
    return (bool) file_unmanaged_move($destination, $permanent_destination, FILE_EXISTS_REPLACE);
  }
  return $success;
}
use lsolesen\pel\PelDataWindow;
use lsolesen\pel\PelJpeg;
use lsolesen\pel\PelExif;
use lsolesen\pel\PelTiff;
use lsolesen\pel\PelIfd;

/**
 * get the exif data from the original file, allow other modules to alter, save
 * to new derivative file
 */
function image_exif_save_metadata($image, $derivative_path) {
  libraries_load('pel');
  if ($wrapper = file_stream_wrapper_get_instance_by_uri($image->source)) {
    $original_path = $wrapper->realpath();
  }
  else {
    return;
  }
  // make sure we capture all output incase the library puts anything out that we don't want
  ob_start();
  $original_image = image_exif_get_ifd($original_path);
  $derivative_image = image_exif_get_ifd($derivative_path);
  $derivative_image->setExif($original_image->getExif());
  $derivative_image->saveFile($derivative_path);
  $out1 = ob_get_contents();
  ob_end_clean();
}

/**
 * get the PelJpeg object for an image;
 * taken mostly from pel/examples/edit-description.php with comments
 */
function image_exif_get_ifd($image_path) {
  $data = new PelDataWindow(file_get_contents($image_path));
  if (PelJpeg::isValid($data)) {
    /*
     * The data was recognized as JPEG data, so we create a new empty
     * PelJpeg object which will hold it. When we want to save the
     * image again, we need to know which object to same (using the
     * getBytes method), so we store $jpeg as $file too.
     */
    $jpeg = $file = new PelJpeg();
    /*
     * We then load the data from the PelDataWindow into our PelJpeg
     * object. No copying of data will be done, the PelJpeg object will
     * simply remember that it is to ask the PelDataWindow for data when
     * required.
     */
    $jpeg->load($data);
    /*
     * The PelJpeg object contains a number of sections, one of which
     * might be our Exif data. The getExif() method is a convenient way
     * of getting the right section with a minimum of fuzz.
     */
    $exif = $jpeg->getExif();
    if ($exif == null) {
        /*
         * Ups, there is no APP1 section in the JPEG file. This is where
         * the Exif data should be.
         *
         * In this case we simply create a new APP1 section (a PelExif
         * object) and adds it to the PelJpeg object.
         */
        $exif = new PelExif();
        $jpeg->setExif($exif);
        /* We then create an empty TIFF structure in the APP1 section. */
        $tiff = new PelTiff();
        $exif->setTiff($tiff);
    } else {
        /*
         * Surprise, surprise: Exif data is really just TIFF data! So we
         * extract the PelTiff object for later use.
         */
        $tiff = $exif->getTiff();
    }
  } elseif (PelTiff::isValid($data)) {
    /*
     * The data was recognized as TIFF data. We prepare a PelTiff
     * object to hold it, and record in $file that the PelTiff object is
     * the top-most object (the one on which we will call getBytes).
     */
    $tiff = $file = new PelTiff();
    /* Now load the data. */
    $tiff->load($data);
  } else {
    /*
     * The data was not recognized as either JPEG or TIFF data.
     * Complain loudly, dump the first 16 bytes, and exit.
     */
    return;
  }

  /*
   * TIFF data has a tree structure much like a file system. There is a
   * root IFD (Image File Directory) which contains a number of entries
   * and maybe a link to the next IFD. The IFDs are chained together
   * like this, but some of them can also contain what is known as
   * sub-IFDs. For our purpose we only need the first IFD, for this is
   * where the image description should be stored.
   */
  $ifd0 = $tiff->getIfd();
  if ($ifd0 == null) {
    /*
     * No IFD in the TIFF data? This probably means that the image
     * didn't have any Exif information to start with, and so an empty
     * PelTiff object was inserted by the code above. But this is no
     * problem, we just create and inserts an empty PelIfd object.
     */
    $ifd0 = new PelIfd(PelIfd::IFD0);
    $tiff->setIfd($ifd0);
  }
  return $file;
}

/**
 * @see image_gd_create_tmp().
 */
function image_exif_create_tmp(stdClass $image, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_create_tmp($image, $width, $height);
}

/**
 * @see image_gd_get_info().
 */
function image_exif_get_info(stdClass $image) {
  module_load_include('inc', 'system', 'image.gd');
  $details = image_gd_get_info($image);
  return $details;
}