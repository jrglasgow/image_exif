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
  $form['status']['#markup'] .= t('EXIF Metadata Retention relies heavily on "GD2 image manipulation toolkit".');
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

/**
 * get the exif data from the original file, allow other modules to alter, save
 * to new derivative file
 */
function image_exif_save_metadata($image, $derivative_path) {
  libraries_load('pel');

  if ($wrapper = file_stream_wrapper_get_instance_by_uri($image->source)) {
    $path = $wrapper->realpath();
  }
  // get the original file exif attributes
  $original_jpeg = new lsolesen\pel\PelJpeg($path);

  $original_data = image_exif_get_data($original_jpeg);

  // allow other modules to alter the data before it goes to the derivative image
  drupal_alter('image_exif', $original_data);


  $derivative_jpeg = new lsolesen\pel\PelJpeg($derivative_path);
  $derivative_jpeg->setExif($original_jpeg->getExif());
  $derivative_jpeg->saveFile($derivative_path);
}

/**
 * get the EXIF data
 */
function image_exif_get_data($jpeg) {

  $exif = $jpeg->getExif();
  if (empty($exif)) {
    return FALSE;
  }
  $tiff = $exif->getTiff();
  $ifd = $tiff->getIfd();
  $data = array();
  image_exif_get_data_recurse($data, $ifd);
  return $data;
}

/**
 * recurse through the tree getting exif data
 */
function image_exif_get_data_recurse(&$parent, $ifd) {
  $parent[$ifd->getName()] = array(
    'this' => $ifd,
  );
  $name = $ifd->getName();
  // get all the entries
  foreach ($ifd as $tag => $entry) {
    $parent[$ifd->getName()][lsolesen\pel\PelTag::getName($entry->getIfdType(), $entry->getTag())] = array(
      'text'  => $entry->getText(),
      'value' => $entry->getValue(),
      'entry' => $entry,
    );
  }
  // get all the sib Ifds
  $subIfds = $ifd->getSubIfds();
  if (!empty($subIfds)) {
    foreach($subIfds AS $sIfd) {
      image_exif_get_data_recurse($parent[$ifd->getName()], $sIfd);
    }
  }
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