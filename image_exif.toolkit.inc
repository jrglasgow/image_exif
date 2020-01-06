<?php

/**
 * @file
 *
 * contain the image toolkit necessary callbacks
 */
use Monolog\Logger;
use PHPExiftool\Reader;
use PHPExiftool\Writer;

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
 * @param $form
 * @param $form_state
 *
 * @see image_gd_settings_validate().
 */
function image_exif_settings_validate($form, &$form_state) {
  module_load_include('inc', 'system', 'image.gd');
  image_gd_settings_validate($form, $form_state);
}

/**
 * @param \stdClass $image
 * @param           $width
 * @param           $height
 *
 * @return bool
 * @see image_gd_resize().
 */
function image_exif_resize(stdClass $image, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_resize($image, $width, $height);
}

/**
 * @param \stdClass $image
 * @param           $degrees
 * @param null      $background
 *
 * @return bool
 * @see image_gd_rotate().
 */
function image_exif_rotate(stdClass $image, $degrees, $background = NULL) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_rotate($image, $degrees, $background = NULL);
}

/**
 * @param \stdClass $image
 * @param           $x
 * @param           $y
 * @param           $width
 * @param           $height
 *
 * @return bool
 * @see image_gd_crop().
 */
function image_exif_crop(stdClass $image, $x, $y, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_crop($image, $x, $y, $width, $height);
}

/**
 * @param \stdClass $image
 *
 * @return bool
 * @see image_gd_desaturate().
 */
function image_exif_desaturate(stdClass $image) {
  return image_gd_desaturate($image);
}

/**
 * @param \stdClass $image
 *
 * @return bool
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
 *
 * @param $image
 * @param $derivative_path
 *
 * @throws \PHPExiftool\Exception\EmptyCollectionException
 */
function image_exif_save_metadata($image, $derivative_path) {
  if ($wrapper = file_stream_wrapper_get_instance_by_uri($image->source)) {
    $original_path = $wrapper->realpath();
  }
  else {
    return;
  }

  $logger = new Logger('exiftool');
  $reader = Reader::create($logger);

  $metadataBag = $reader->files($original_path)->first();
  $writer = Writer::create($logger);
  $writer->write($derivative_path, $metadataBag);
}

/**
 * @param \stdClass $image
 * @param           $width
 * @param           $height
 *
 * @return false|resource
 * @see image_gd_create_tmp().
 */
function image_exif_create_tmp(stdClass $image, $width, $height) {
  module_load_include('inc', 'system', 'image.gd');
  return image_gd_create_tmp($image, $width, $height);
}

/**
 * @param \stdClass $image
 *
 * @return array|bool
 * @see image_gd_get_info().
 */
function image_exif_get_info(stdClass $image) {
  module_load_include('inc', 'system', 'image.gd');
  $details = image_gd_get_info($image);
  return $details;
}
